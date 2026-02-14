#include "chat-client.hpp"
#include "http-client.hpp"
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

ChatClient::ChatClient(const std::string& endpoint,
                       const std::string& api_key_env_var,
                       const std::string& model,
                       const std::string& system_prompt)
  : endpoint_(endpoint), model_(model), system_prompt_(system_prompt),
    http_client_(std::make_unique<HttpClient>())
{
  if (!api_key_env_var.empty())
  {
    const char* key = std::getenv(api_key_env_var.c_str());
    if (!key)
    {
      throw std::runtime_error("Environment variable not set: " +
                               api_key_env_var);
    }
    api_key_ = key;
  }
}

ChatClient::~ChatClient() = default;

std::string ChatClient::send_message(const std::string& user_message)
{
  history_.push_back({"user", user_message});

  std::string request_body = build_request_body();

  std::map<std::string, std::string> headers;
  headers["Content-Type"] = "application/json";
  if (!api_key_.empty())
  {
    headers["Authorization"] = "Bearer " + api_key_;
  }

  std::string response = http_client_->post(endpoint_, request_body, headers);
  std::string assistant_message = parse_response(response);

  history_.push_back({"assistant", assistant_message});

  return assistant_message;
}

void ChatClient::clear_history()
{
  history_.clear();
}

std::string ChatClient::build_request_body() const
{
  json request;
  request["model"] = model_;

  json input = json::array();

  if (!system_prompt_.empty())
  {
    json system_msg;
    system_msg["role"] = "system";
    system_msg["content"] = system_prompt_;
    input.push_back(system_msg);
  }

  for (const auto& msg : history_)
  {
    json message;
    message["role"] = msg.role;
    message["content"] = msg.content;
    input.push_back(message);
  }

  request["messages"] = input;

  return request.dump();
}

std::string ChatClient::parse_response(const std::string& response) const
{
  json j = json::parse(response);

  if (j.contains("error") && !j["error"].is_null())
  {
    throw std::runtime_error(j["error"].dump());
  }

  return j["choices"][0]["message"]["content"].get<std::string>();
}
