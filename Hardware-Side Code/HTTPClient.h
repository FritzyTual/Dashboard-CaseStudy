#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <WiFiS3.h>

class HTTPClient
{
  private:
    WiFiSSLClient client;
    String host;
    String endpoint;
    int port;
    std::vector<String> headers;
    String body;

    // gpt Disable copy constructor and assignment operator // reducing cost
    HTTPClient(const HTTPClient&) = delete;
    HTTPClient& operator=(const HTTPClient&) = delete;

  public:
    HTTPClient();
    ~HTTPClient();

    void begin(const WiFiSSLClient &sslClient, const String &url, const int& nport);
    void setHeader(const String &content);
    void setTimeout(const int &ms);
    int POST(const String &requestBody);
    int GET();
    String getBody() const;
    void close();
};

#endif  //HTTP_CLIENT_H