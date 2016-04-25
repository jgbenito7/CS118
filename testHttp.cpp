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

  response.setData("<html><body><h1>Yo whats up</h1></body></html>");
  ByteBlob b = response.encode();
  HttpResponse decoded = response.decode(b);

  cout << "First Line: HTTP/" << decoded.getVersion() << " " << decoded.getStatus()<< endl;
  decoded.printHeader();
  cout << "Data: " << decoded.getData() << endl;

  /*request.setUrl("www.test.com/test");
  //
  request.setHeader("Accept","text/html,application/xhtml+xml");
  request.setHeader("Accept-Language","en-us,en;q=0.5");
  ByteBlob b = request.encode();
  HttpRequest decoded = request.decode(b);
  cout << "First Line: " << decoded.getMethod() << " " << decoded.getUrl() << " HTTP/" << decoded.getVersion() << endl;
  decoded.printHeader();*/
}
