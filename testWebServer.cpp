#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "http.h"
#include <thread>

using namespace std;

void processResponse(HttpResponse resp){
  if(resp.getStatus() == HttpResponse::OK_200){

  }
}

int doTest(){
  HttpRequest request;
  request.setUrl("./server/index.html");
  request.setHeader("Accept","text/html,application/xhtml+xml");
  request.setHeader("Accept-Language","en-us,en;q=0.5");
  ByteBlob reqByteBlob = request.encode();

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(40000);     // short, network byte order
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }

  char ipstr[INET_ADDRSTRLEN] = { '\0' };
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // send request
  std::string reqStr(reqByteBlob.begin(), reqByteBlob.end());
  if (send(sockfd, reqStr.c_str(), reqStr.size(), 0) == -1) {
    perror("send");
    return 4;
  }

  //receive response
  bool isEnd = false;
  char buf[20] = { 0 };
  stringstream ssOverall;
	stringstream ssIteration;
	const string endingStr = "\r\n\r\n";
	unsigned int endingCount = 0;
  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    if (recv(sockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }
    ssOverall << buf;
		ssIteration << buf;
		string currString = ssIteration.str();

    for(unsigned int i = 0; i < currString.length(); i++){
			if(currString[i] == endingStr[endingCount])
				endingCount++;
			else
				endingCount = 0;
			if(endingCount == 4){
				string totalRespString = ssOverall.str();
				vector<uint8_t> decoded(totalRespString.begin(), totalRespString.end());
				//HttpResponse resp = HttpResponse::decode((ByteBlob)decoded);
				//processResponse(resp);

				ssOverall.str("");
				endingCount = 0;

        isEnd = true;
			}
		}
  }

  close(sockfd);

  return 0;
}

int main(){
  thread(doTest).detach();
  doTest();
}
