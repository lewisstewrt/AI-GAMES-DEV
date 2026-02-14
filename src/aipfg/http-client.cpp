#include "http-client.hpp"
#include <curl/curl.h>
#include <stdexcept>

namespace
{

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  auto* response = static_cast<std::string*>(userdata);
  response->append(ptr, size * nmemb);
  return size * nmemb;
}

} // anonymous namespace

HttpClient::HttpClient()
{
  curl_handle_ = curl_easy_init();
  if (!curl_handle_)
  {
    throw std::runtime_error("Failed to initialize CURL");
  }
}

HttpClient::~HttpClient()
{
  if (curl_handle_)
  {
    curl_easy_cleanup(static_cast<CURL*>(curl_handle_));
  }
}

std::string HttpClient::post(const std::string& url, const std::string& body,
                             const std::map<std::string, std::string>& headers)
{
  CURL*       curl = static_cast<CURL*>(curl_handle_);
  std::string response;

  curl_easy_reset(curl);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  struct curl_slist* header_list = nullptr;
  for (const auto& header : headers)
  {
    std::string header_str = header.first + ": " + header.second;
    header_list = curl_slist_append(header_list, header_str.c_str());
  }
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

  CURLcode res = curl_easy_perform(curl);
  curl_slist_free_all(header_list);

  if (res != CURLE_OK)
  {
    throw std::runtime_error(std::string("CURL request failed: ") +
                             curl_easy_strerror(res));
  }

  return response;
}
