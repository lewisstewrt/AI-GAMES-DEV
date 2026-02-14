#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <map>
#include <string>

class HttpClient
{
public:
  HttpClient();
  ~HttpClient();

  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;

  std::string post(const std::string& url, const std::string& body,
                   const std::map<std::string, std::string>& headers);

private:
  void* curl_handle_;
};

#endif // HTTP_CLIENT_HPP
