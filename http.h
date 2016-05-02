#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

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
    ByteBlob m_data;

  public:
    const static HttpStatus OK_200;
    const static HttpStatus BR_400;
    const static HttpStatus NF_404;

    HttpResponse(){};
    //virtual void decodeFirstLine(ByteBlob line);
    HttpStatus getStatus();
    void setStatus(string status);
    string getDescription();
    void setDescription(string description);
    void setData(ByteBlob data);
    void printHeader();
    ByteBlob getData();
    ByteBlob encode();
    static HttpResponse decode(ByteBlob response);

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
        vector<string> headerStrings;
        istringstream ss(token);
        string s;
        string colon = ":";

        bool foundString = false;

        while(getline(ss,s,':')){
          foundString = true;
          //cout << itr << ": " << s << endl;
          headerStrings.push_back(s);
        }
        if(foundString){
          //cout << headerStrings[0] << endl;
          string modify = headerStrings.at(1).erase(0,1);
          httpR.setHeader(headerStrings.at(0),modify);
        }
      }
      decoded.erase(0, pos + delimiter.length());
      itr++;
  }
  return httpR;
}

//////////////////////////////////////////////
// HttpResponse Declarations
//////////////////////////////////////////////

const HttpStatus HttpResponse::OK_200 = "200 OK";
const HttpStatus HttpResponse::BR_400 = "400 Bad request";
const HttpStatus HttpResponse::NF_404 = "404 Not found";

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

void HttpResponse::setData(ByteBlob data){
  m_data = data;
}

ByteBlob HttpResponse::getData(){
  return m_data;
}

void HttpResponse::printHeader(){
  string request = "";
  map<string, string> m = getHeaderMap();
  for(map<string,string>::iterator it = m.begin(); it != m.end(); ++it) {
    cout << it->first << ": " << m[it->first] << "\n";
  }
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
  httpString = firstLine + header + "\r\n";
  //Encode the http header as a byteblob
  vector<uint8_t> encoded(httpString.begin(),httpString.end());
  vector<uint8_t> data = getData();
  //Append the header blob to the data blob and return
  encoded.insert(encoded.end(), data.begin(), data.end());
  return (ByteBlob) encoded;
}

HttpResponse HttpResponse::decode(ByteBlob response){
  ///////////////////////////////////////////////////////
  // First Find Where The Header Ends and Store the String
  ///////////////////////////////////////////////////////

  int crlfCount = 0;
  int charCount = 0;
  while(true){
    char c1 = response.at(charCount);
    char c2 = response.at(charCount + 1);
    // cout << (int) response.at(charCount) << (int) response.at(charCount + 1) << endl;

    if(c1 == '\r' && c2 == '\n'){
      crlfCount++;
      //cout << "Found one" << endl;
    }else if((c1 == '\n' && c2 == '\r')&&(crlfCount==1)){
      //cout << "Chill" << endl;
    }else{
      crlfCount = 0;
    }

    if(crlfCount == 2){
      charCount+=2;
      break;
    }

    charCount+=1;
  }

  //Save the http header as a string
  string decoded = "";
  for(int i=0; i<charCount; i++){
    decoded+=response.at(i);
  }

  ///////////////////////////////////////////////////////
  // Parse the header string to extract info
  ///////////////////////////////////////////////////////

  // cout << decoded;

  string delimiter = "\r\n";
  HttpResponse httpR;
  int itr = 0;
  size_t pos = 0;
  string token;
  bool headerEnd = false;
  ByteBlob data;

  while ((pos = decoded.find(delimiter)) != string::npos) {
      //Extract the line
      token = decoded.substr(0, pos);

      if(token==""){
        //cout << "blank string" << endl;
      }else{
        //cout << "token: " << token << endl;
      }

      //decode the first line
      if(itr == 0){
        size_t first_line_pos = 0;
        string space = " ";
        string first_line_token;
        int linePiece = 0;

        while ((first_line_pos = token.find(space)) != string::npos) {
          //Get a piece of the first line
          first_line_token = token.substr(0, first_line_pos);

          //See what the status is
          if(linePiece==1){
            if(first_line_token=="200"){
              httpR.setStatus(HttpResponse::OK_200);
            }else if(first_line_token=="400"){
              httpR.setStatus(HttpResponse::BR_400);
            }else if(first_line_token=="404"){
              httpR.setStatus(HttpResponse::NF_404);
            }
          }
          linePiece++;
          token.erase(0, first_line_pos + space.length());
        }
      }

      //Check if we reached the end of the header section
      if(token==""){
        headerEnd = true;
      }

      //Decode Header Files
      if(itr>0 && !headerEnd){
        vector<string> headerStrings;
        istringstream ss(token);
        string s;
        string colon = ":";

        bool foundString = false;

        while(getline(ss,s,':')){
          foundString = true;
          headerStrings.push_back(s);
        }
        if(foundString){
          string modify = headerStrings.at(1).erase(0,1);
          httpR.setHeader(headerStrings.at(0),modify);
        }
      }else if(headerEnd){
        //Save the remainder of the response as a byteblob of data
        for(ByteBlob::iterator x=(response.begin()+charCount); x<response.end(); x++){
          data.push_back(*x);
        }
        break;
      }

      decoded.erase(0, pos + delimiter.length());
      itr++;
  }
  httpR.setData(data);
  return httpR;
}
