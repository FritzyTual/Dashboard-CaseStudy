#include "HTTPClient.h"

HTTPClient::HTTPClient() : port(0), body("") {}

HTTPClient::~HTTPClient()
{
    this->client.flush();
    this->close();
}

/*
 * @params: sslClient, url, nport
 * @Description: Initializes client, url, port. Then, extracts the url to get the host, then the endpoint in which where it will do the post request
 */
void HTTPClient::begin(const WiFiSSLClient &sslClient, const String &url, const int& nport)
{
    this->client = sslClient;
    this->port = nport;

    // extract the host and endpoint from the URL
    int index = url.indexOf("//") + 2; // skip "https://"
    int endpointIndex = url.indexOf("/", index);

    this->host = url.substring(index, endpointIndex);
    this->endpoint = url.substring(endpointIndex);
}

/*
 * @params: content
 * @Description: Initializes the Content Headers to make the request connection.
 */
void HTTPClient::setHeader(const String &content)
{
  this->headers.push_back(content);
}

/*
 * @params: ms
 * @Description: set timeout when request does not respond within specific time.
 */
void HTTPClient::setTimeout(const int &ms)
{
  this->client.setTimeout(ms);
}

/*
 * @params: requestBody
 * @Description: called by reference base on declared data to post request, then wait for response, then if responded, break, then init response body
 * then clear content headers to prevent additional headers from previous request.
 */
int HTTPClient::POST(const String &requestBody)
{
    if (this->client.connect(this->host.c_str(), this->port))
    {
        this->client.println("POST " + endpoint + " HTTP/1.1");
        this->client.println("Host: " + host);
        for (const String& header : headers)
          this->client.println(header);
        this->client.println("Content-Length: " + String(requestBody.length()));
        this->client.println();
        this->client.print(requestBody);

        // wait for the server response
        int responseCode = -1;
        while (this->client.connected())
        {
          String line = this->client.readStringUntil('\n');
          if (line.startsWith("HTTP/1.1 "))
            responseCode = line.substring(9, 12).toInt(); // response code

          if (line == "\r")
            break;
        }
        this->headers.clear(); // clear previous cheaders
        this->body = client.readStringUntil('\n');

        return responseCode;
    } else {
        return -1;
    }
}

int HTTPClient::GET()
{
  if (this->client.connect(this->host.c_str(), this->port))
  {
    this->client.println("GET " + endpoint + " HTTP/1.1");
    this->client.println("Host: " + host);
    for (const String& header : headers)
      this->client.println(header);
    this->client.println();

    int responseCode = -1;
    while (this->client.connected())
    {
        String line = this->client.readStringUntil('\n');
        if (line.startsWith("HTTP/1.1 "))
          responseCode = line.substring(9, 12).toInt();

        if (line == "\r")
          break;
    }
    this->headers.clear();
    this->body = client.readStringUntil('\n');

    return responseCode;
  } else {
      return -1;
  }
}

/*
 * @Description: Initialized through client.readStringUntil('\n'); to get the response body after request
 */
String HTTPClient::getBody() const
{
  return this->body;
}

/*
 * @Description: Close connection to prevent resource leaks
 */
void HTTPClient::close()
{
  this->client.stop();
}