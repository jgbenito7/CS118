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
typedef string HttpVersion;
typedef string HttpStatus;

class HttpMessage{
  private:
    HttpVersion m_version;
    map<string,string> m_headers;
    ByteBlob payload;

  public:
    HttpMessage()
      : m_version("1.0")
      {}
    void setHeader(string key, string value);
    string getHeader(string key);
    void decodeHeaderLine(ByteBlob line);
    void setPayLoad(ByteBlob blob);
    HttpVersion getVersion();
    ByteBlob getPayload();
    map<string,string> getHeaderMap();
};


class HttpRequest : public HttpMessage{
  private:
    string m_method;
    string m_url;

  public:
    HttpRequest() :
      HttpMessage(), m_method("GET"){}
    string getUrl();
    void setUrl(string url);
    string getMethod();
    void printHeader();
    ByteBlob encode();
};

class HttpResponse : public HttpMessage {
  private:
    HttpStatus m_status;
    string m_statusDescription;
    string m_data;

  public:
    HttpResponse();
    //virtual void decodeFirstLine(ByteBlob line);
    HttpStatus getStatus();
    void setStatus(string status);
    string getDescription();
    void setDescription(string description);
    void setData(string data);
    string getData();
    ByteBlob encode();

};

//////////////////////////////////////////////
// HttpMessage Declarations
//////////////////////////////////////////////
HttpVersion HttpMessage::getVersion(){
  return m_version;
}

void HttpMessage::setHeader(string key, string value){
  m_headers[key] = value;
}

string HttpMessage::getHeader(string key){
  return m_headers[key];
}

map<string, string> HttpMessage::getHeaderMap(){
  return m_headers;
}

//////////////////////////////////////////////
// HttpRequest Declarations
//////////////////////////////////////////////

void HttpRequest::setUrl(string url){
  m_url = url;
}

string HttpRequest::getUrl(){
  return m_url;
}

string HttpRequest::getMethod(){
  return m_method;
}

void HttpRequest::printHeader(){
  string request = "";
  map<string, string> m = getHeaderMap();
  for(map<string,string>::iterator it = m.begin(); it != m.end(); ++it) {
    cout << it->first << ": " << m[it->first] << "\n";
  }
}

ByteBlob HttpRequest::encode(){
  //Build the strings
  string httpString;
  string firstLine = getMethod() + " " + getUrl() + " HTTP/" + getVersion() + "\r\n";
  string header = "";
  map<string, string> m = getHeaderMap();
  for(map<string,string>::iterator it = m.begin(); it != m.end(); ++it) {
    header += it->first + ": " + m[it->first] + "\r\n";
  }
  httpString = firstLine + header + "\r\n";

  vector<uint8_t> encoded(httpString.begin(),httpString.end());
  return (ByteBlob) encoded;
}

//////////////////////////////////////////////
// HttpResponse Declarations
//////////////////////////////////////////////

void HttpResponse::setStatus(string status){
  m_status = status;
}

HttpStatus HttpResponse::getStatus(){
  return m_status;
}

void HttpResponse::setDescription(string description){
  m_statusDescription = description;
}

HttpStatus HttpResponse::getDescription(){
  return m_statusDescription;
}

void HttpResponse::setData(string data){
  m_data = data;
}

string HttpResponse::getData(){
  return m_data;
}

ByteBlob HttpResponse::encode(){
  //Build the strings
  string httpString;
  string firstLine = "HTTP/" + getVersion() + " " + getStatus() + "\r\n";
  string header = "";
  map<string, string> m = getHeaderMap();
  for(map<string,string>::iterator it = m.begin(); it != m.end(); ++it) {
    header += it->first + ": " + m[it->first] + "\r\n";
  }
  httpString = firstLine + header + "\r\n" + getData();
  vector<uint8_t> encoded(httpString.begin(),httpString.end());
  return (ByteBlob) encoded;
}


int main(){
  HttpRequest request;
  request.setUrl("www.test.com/test");
  cout << "First Line: " << request.getMethod() << " " << request.getUrl() << " HTTP/" << request.getVersion() << endl;
  request.setHeader("Accept","text/html,application/xhtml+xml");
  request.setHeader("Accept-Language","en-us,en;q=0.5");
  ByteBlob b = request.encode();
  cout << b.at(1) << endl;
}
