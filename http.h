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
    static HttpRequest decode(ByteBlob request);
};

class HttpResponse : public HttpMessage {
  private:
    HttpStatus m_status;
    string m_statusDescription;
    string m_data;

  public:
    static const HttpStatus 200_OK = "200 OK";
    static const HttpStatus 400_BR = "400 Bad request";
    static const HttpStatus 404_NF = "404 Not found";

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

HttpRequest HttpRequest::decode(ByteBlob request){
  string decoded(request.begin(),request.end());
  string delimiter = "\r\n";

  HttpRequest httpR;

  int itr = 0;

  size_t pos = 0;
  string token;
  while ((pos = decoded.find(delimiter)) != string::npos) {
      //Extract the line
      token = decoded.substr(0, pos);

      //decode the first line
      if(itr == 0){
        size_t first_line_pos = 0;
        string space = " ";
        string first_line_token;
        int linePiece = 0;

        while ((first_line_pos = token.find(space)) != string::npos) {
          //Get a piece of the first line
          first_line_token = token.substr(0, first_line_pos);
          if(linePiece==1){
            //Set the url
            httpR.setUrl(first_line_token);
          }
          linePiece++;
          token.erase(0, first_line_pos + space.length());
        }
      }

      //Decode Header Files
      if(itr>0){
        //size_t header_pos = 0; //Commented out because unused
        string colon = " ";
        string end = "\r\n";
        cout << token << endl;
        string first = token.substr(0,token.find(colon));
        //string second = token.substr(token.find(colon)+2,token.find(end));
        httpR.setHeader(first,"test");
      }

      decoded.erase(0, pos + delimiter.length());
      itr++;
  }
  return httpR;
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
