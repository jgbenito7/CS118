#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <map>

using namespace std;

/////////////////////////
// HttpRequest needs:
// Method : URL : Version
// List of headers
// optional Payload

typedef vector<uint8_t> ByteBlob;

class HttpMessage{
private:
  string HttpVersion; //Probably dont need this
  map<string,string> m_headers;
  string payload;
public:
  virtual void decodeFirstLine(ByteBlob line) = 0;
  string getVersion(); //Probably dont need this
  void setHeader(string key, string value);
  string getHeader(string key);
  void decodeHeaderLine(ByteBlob line);
  void setPayLoad(ByteBlob blob);
  ByteBlob getPayload();
}


class HttpRequest : HttpMessage{
  private:
    string m_method;
    string m_url;

  public:
    virtual void decodeFirstLine(ByteBlob line);
    string getMethod(); //Probably dont need this
    void setMethod(string method); //Probably dont need this
    string getUrl();
    void setUrl(string url);
};

class HttpResponse : HttpMessage {
  private:
    string m_status;
    string m_statusDescription;

  public:
    virtual void decodeFirstLine(ByteBlob line);
    string getStatus();
    void setStatus(string status);
    string getDescription();
    void setDescription(string description);

}

int main(){

}
