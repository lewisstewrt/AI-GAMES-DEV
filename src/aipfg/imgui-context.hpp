#ifndef _IMGUI_CONTEXT_HPP_
#define _IMGUI_CONTEXT_HPP_

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

// RAII wrapper for Dear ImGui
class ScopedImGui
{
public:
  ScopedImGui(SDL_Window* window, SDL_Renderer* renderer,
              float font_size = 32.0f)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    // Six fonts in ImGui: Roboto-Medium Cousine-Regular DroidSans
    //                     Karla-Regular ProggyClean ProggyTiny
    io.Fonts->AddFontFromFileTTF(IMGUI_FONT_DIR "Cousine-Regular.ttf",
                                 font_size);

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
  }

  ScopedImGui(const ScopedImGui&) = delete;
  ScopedImGui& operator=(const ScopedImGui&) = delete;

  ~ScopedImGui()
  {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
};

#endif // _IMGUI_CONTEXT_HPP_
