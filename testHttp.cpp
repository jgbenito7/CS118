#include <iostream>
#include <sstream>

#include "http.h"

using namespace std;

int main(){
  HttpResponse response;

  response.setStatus(HttpResponse::OK_200);
  response.setHeader("Date","Fri, 08 Aug 2003 08:12:31 GMT");
  response.setHeader("A","Test");
  response.setHeader("Content-Type", "text/html");

  string dataS = string("<html>") + "\r\n" + "<body><h1>Yo whats up</h1></body></html>" + "\r\n";
  vector<uint8_t> data(dataS.begin(),dataS.end());
  response.setData(data);
  ByteBlob b = response.encode();
  HttpResponse decoded = response.decode(b);
  ByteBlob d = decoded.getData();

  for(ByteBlob::iterator x=d.begin(); x<d.end(); x++){
    cout << *x;
    //data.push_back(*x);
  }

  //cout << "First Line: HTTP/" << decoded.getVersion() << " " << decoded.getStatus()<< endl;
  //decoded.printHeader();
  //cout << "Data: " << decoded.getData() << endl;

  /*request.setUrl("www.test.com/test");
  //
  request.setHeader("Accept","text/html,application/xhtml+xml");
  request.setHeader("Accept-Language","en-us,en;q=0.5");
  ByteBlob b = request.encode();
  HttpRequest decoded = request.decode(b);
  cout << "First Line: " << decoded.getMethod() << " " << decoded.getUrl() << " HTTP/" << decoded.getVersion() << endl;
  decoded.printHeader();*/
}
