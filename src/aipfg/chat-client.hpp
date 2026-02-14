#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <memory>
#include <string>
#include <vector>

class HttpClient;

struct Message
{
  std::string role;
  std::string content;
};

class ChatClient
{
public:
  ChatClient(const std::string& endpoint, const std::string& api_key_env_var,
             const std::string& model, const std::string& system_prompt = "");

  ~ChatClient();

  ChatClient(const ChatClient&) = delete;
  ChatClient& operator=(const ChatClient&) = delete;

  std::string send_message(const std::string& user_message);

  void clear_history();

private:
  std::string                 endpoint_;
  std::string                 api_key_;
  std::string                 model_;
  std::string                 system_prompt_;
  std::vector<Message>        history_;
  std::unique_ptr<HttpClient> http_client_;

  std::string build_request_body() const;
  std::string parse_response(const std::string& response) const;
};

#endif // CHAT_CLIENT_HPP
