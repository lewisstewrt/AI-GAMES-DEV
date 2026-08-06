// Banner IDs: B01590256 Lewis Stewart


#include "aipfg/chat-client.hpp"
#include "aipfg/imgui-context.hpp"
#include "aipfg/sdl3-context.hpp"
#include "aipfg/sdl3-typedefs.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// async LLM call so there is no freezing (Task 8 )
#include <future>
#include <chrono>

#ifdef _WIN32
#include <windows.h> // SetConsoleOutputCP & SetConsoleCP for unicode on cmd.exe
#endif

class Game
{
public:
    explicit Game(SDLContext&, int w, int h)
        : window_{ SDL_CreateWindow("LLM Game", w, h, SDL_WINDOW_RESIZABLE),
                  SDL_DestroyWindow },
        renderer_{ SDL_CreateRenderer(window_.get(), nullptr), SDL_DestroyRenderer },
        imgui_ctx_{ nullptr },

        // Polaris config + system prompt for Tic Tac Toe (Task 3)
        // We use the same endpoint/model as the starter chatbot but different system prompt.
        chat_client_{
          "https://polaris.uws.ac.uk/api/chat/completions",
          "POLARIS_API_KEY",
          "gpt-oss:20b",
          "You are playing Tic Tac Toe.\n"
          "You are O.\n"
          "You will receive a 9-character board string.\n"
          "Spaces mean empty cells.\n"
          "Reply with ONLY one digit 0-8 for your move.\n"
          "Index order is left-to-right, top-to-bottom."
        }
    {
        if (!window_ || !renderer_)
        {
            throw std::runtime_error(SDL_GetError());
        }

        SDL_SetRenderVSync(renderer_.get(), 1);

        float font_size = h / 25.0f;
        imgui_ctx_ =
            std::make_unique<ScopedImGui>(window_.get(), renderer_.get(), font_size);

        input_buffer_[0] = '\0';
    }

    void run()
    {
        bool running = true;
        while (running)
        {
            process_events(running);
            update();
            render();
        }
    }

private:
    // Tic Tac Toe game logic 

    // checks if someone won, draw or still playing (Task 4)
    // returns: 'X' human win, 'O' LLM win, 'D' draw, ' ' still playing
    char check_winner() const
    {
		// all winning lines for rows, columns and diagonals
        const int L[8][3] = {
          {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
          {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // columns
          {0, 4, 8}, {2, 4, 6}             // diagonals
        };

        for (const auto& line : L)
        {
            char a = board_[line[0]];
            if (a != ' ' && a == board_[line[1]] && a == board_[line[2]])
            {
                return a;
            }
        }

        // draw check if theres no empty spaces
        bool empty = false;
        for (char c : board_)
            if (c == ' ')
                empty = true;

        return empty ? ' ' : 'D';
    }

    // end game and update score in one place (Task 5)
    void finish_game(char w)
    {
        game_over_ = true;
        winner_ = w;

        if (w == 'X') wins_++;
        else if (w == 'O') losses_++;
        else if (w == 'D') draws_++;
    }

    //convert board to a 9 character string for the LLM (Task 3)
    std::string encode_board() const
    {
        return std::string(board_.begin(), board_.end());
    }

    //pull the first digit from 0 to 8 out of the LLM reply (Task 3)
    int parse_move(const std::string& s) const
    {
        for (char c : s)
            if (c >= '0' && c <= '8')
                return c - '0';
        return -1;
    }

    //if LLM reply is invalid (choose first empty)
    int first_empty() const
    {
        for (int i = 0; i < 9; i++)
            if (board_[i] == ' ')
                return i;
        return -1;
    }

    //reset game and alternate who starts (Task 4)
    void reset_game()
    {
        std::fill(board_.begin(), board_.end(), ' ');
        game_over_ = false;
        winner_ = ' ';

        //if we reset while waiting then ignore the pending reply
        llm_thinking_ = false;

        // alternate who goes first each new game
        starting_human_ = !starting_human_;
        human_turn_ = starting_human_;
    }

   

    void process_events(bool& running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);

            if (e.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            else if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE)
            {
                running = false;
            }
        }
    }

    void update()
    {
        const float t = SDL_GetTicks() / 1000.0f;
        const float pi = 3.14159265f;
        bg_r_ = 0.3f + 0.3f * std::sin(t * 3.0f);
        bg_g_ = 0.3f + 0.3f * std::sin(t * 3.0f + pi * (2.0f / 3.0f));
        bg_b_ = 0.3f + 0.3f * std::sin(t * 3.0f + pi * (4.0f / 3.0f));

        // Start async LLM request so UI doesn't freeze (Task 8)
        // We only start it once when it becomes the LLM's turn
        if (!game_over_ && !human_turn_ && !llm_thinking_)
        {
            llm_thinking_ = true;

            // send board state to LLM (Task 3)
            std::string board_str = encode_board();

            llm_future_ = std::async(std::launch::async, [this, board_str]()
                {
                    // synchronous call but running in background thread
                    return chat_client_.send_message(board_str);
                });
        }

        // Poll for reply every frame with no freezing(Task 8)
        if (llm_thinking_)
        {
            if (llm_future_.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
            {
                std::string reply = llm_future_.get();
                llm_thinking_ = false;

                // parse and validate LLM move (Task 3)
                int idx = parse_move(reply);

                // if invalid then fallback to first empty
                if (idx < 0 || idx > 8 || board_[idx] != ' ')
                    idx = first_empty();

                if (idx != -1)
                    board_[idx] = 'O';

                // check result after LLM move (Task 4)
                char w = check_winner();
                if (w != ' ')
                    finish_game(w);
                else
                    human_turn_ = true; // back to human
            }
        }
    }

    void render()
    {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        //game window instead of chat window (Task 1)
        ImGui::Begin("LLM Board Game");

        ImGui::Text("Tic Tac Toe");

        //show whose turn it is (Task 5)
        ImGui::Text("Turn: %s", human_turn_ ? "Human (X)" : "LLM (O)");

        //running score display (Task 5)
        ImGui::Text("Score: W %d  L %d  D %d", wins_, losses_, draws_);

        //thinking indicator while waiting for LLM (Task 5)
        if (llm_thinking_)
        {
            ImGui::Text("LLM: Thinking...");
        }

        //show result when game ends (Task 4)
        if (game_over_)
        {
            if (winner_ == 'X') ImGui::Text("Result: You win!");
            else if (winner_ == 'O') ImGui::Text("Result: LLM wins!");
            else if (winner_ == 'D') ImGui::Text("Result: Draw!");
        }

        ImGui::Spacing();

        //board rendering using ImGui buttons (Task 1)
        const float cell_size = 80.0f;

        //loop makes 3x3 grid of buttons
        for (int row = 0; row < 3; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                int idx = row * 3 + col;
                char piece = board_[idx];

                //label uses "##" so each button has a unique ID in ImGui
                std::string label;
                label += (piece == ' ' ? ' ' : piece);
                label += "##";
                label += std::to_string(idx);

                //prevent invalid moves (Task 2)
                bool occupied = (board_[idx] != ' ');
                ImGui::BeginDisabled(game_over_ || occupied || !human_turn_);

                //human move on click (Task 2)
                if (ImGui::Button(label.c_str(), ImVec2(cell_size, cell_size)))
                {
                    board_[idx] = 'X';

                    //check result after human move (Task 4)
                    char w = check_winner();
                    if (w != ' ')
                    {
                        finish_game(w);
                    }
                    else
                    {
                        // switch to LLM turn (Task 5)
                        human_turn_ = false;
                    }
                }

                ImGui::EndDisabled();

                if (col < 2)
                    ImGui::SameLine();
            }
        }

        ImGui::Spacing();

        //new game button (Task 4)
        if (ImGui::Button("New Game"))
        {
            reset_game();
        }

        ImGui::End();

        ImGui::Render();

        
        SDL_SetRenderDrawColorFloat(renderer_.get(), bg_r_, bg_g_, bg_b_, 1.0f);
        SDL_RenderClear(renderer_.get());

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_.get());

        SDL_RenderPresent(renderer_.get());
    }

private:
    WindowPtr                    window_;
    RendererPtr                  renderer_;
    std::unique_ptr<ScopedImGui> imgui_ctx_;

    
    ChatClient               chat_client_;
    std::vector<std::string> chat_history_;
    char                     input_buffer_[256];

    // Tic Tac Toe board state (Task 1/2)
    // 0 1 2
    // 3 4 5
    // 6 7 8
    std::array<char, 9> board_ = { ' ',' ',' ',' ',' ',' ',' ',' ',' ' };

    //turn tracking and game state (Task 4/5)
    bool human_turn_ = true;
    bool game_over_ = false;
    char winner_ = ' ';
    bool starting_human_ = true;

    //async state for LLM (Task 8)
    bool llm_thinking_ = false;
    std::future<std::string> llm_future_;

    //running score (Task 5)
    int wins_ = 0;
    int losses_ = 0;
    int draws_ = 0;

   
    float bg_r_ = 0.1f;
    float bg_g_ = 0.1f;
    float bg_b_ = 0.2f;
};

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    try
    {
        SDLContext sdl;
        Game game{ sdl, 800, 600 };
        game.run();
    }
    catch (const std::exception& e)
    {
        SDL_Log("Fatal error: %s", e.what());
        return -1;
    }

    return 0;
}
