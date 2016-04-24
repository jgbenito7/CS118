#include <iostream>
#include <sstream>

#include "http.h"

using namespace std;

int main(){
  HttpRequest request;
  request.setUrl("www.test.com/test");
  //
  request.setHeader("Accept","text/html,application/xhtml+xml");
  request.setHeader("Accept-Language","en-us,en;q=0.5");
  ByteBlob b = request.encode();
  HttpRequest decoded = request.decode(b);
  cout << "First Line: " << decoded.getMethod() << " " << decoded.getUrl() << " HTTP/" << decoded.getVersion() << endl;
  decoded.printHeader();
}
