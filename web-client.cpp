#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>

#include "http.h"
#include "showip.h"

int main(int argc, char* argv[])
{
  if(argc != 2) {
    perror("Incorrect number of arguments. There should be exactly one argument: Resource URL\n");
    return 6;
  }
  
  // parse for URL and other args
  std::string url = argv[1];
  
  int beginning = int(url.find("//"));
  int endofhost = int(url.find(":", beginning));
  int endofport = int(url.find("/", beginning + 2));
  int len = int(url.size());
  
  std::string hostN;
  std::string portN;
  std::string requestN;
  
  // maybe error checking here?
  hostN = url.substr(beginning + 2, endofhost - beginning - 2);
  portN = url.substr(endofhost + 1, endofport - endofhost - 1);
  requestN = url.substr(endofport, len - endofport);
  
  // checking for default values
  if (endofhost == -1) {
    endofhost = endofport;
    portN = "80";
    hostN = url.substr(beginning + 2, endofhost - beginning - 2);
  }
  
  if (endofport == -1) {
    requestN = "/index.html";
  }
  
  if (beginning == -1) {
    if (endofhost == -1)
      hostN = url.substr(0, len);
    else
      hostN = url.substr(0, endofhost + 1);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    // stuff from sample code
    // gives a socket an IPv4 socket address to communicate with other hosts over TCP/IP network
    
    // set server stuff
    std::string hostIP = getIP(const_cast<char*>(hostN.c_str()));
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(portN.c_str()));     // short, network byte order
    serverAddr.sin_addr.s_addr = inet_addr(hostIP.c_str());
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    
    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
      perror("connect");
      return 2;
    }
    
    // set client stuff
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
      perror("getsockname");
      return 3;
    }
    
    char ipstr[INET_ADDRSTRLEN] = { '\0' };
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    
    // connection should be set up
    std::cout << "Set up a connection from: " << ipstr << ":" <<
      ntohs(clientAddr.sin_port) << std::endl;
    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    
    // set up HttpRequest
    HttpRequest request;
    request.setUrl(requestN);
    request.setHeader("Accept", "text/html,application/xhtml+xml");
    request.setHeader("Accept-Language", "en-us,en;q=0.5");
    ByteBlob requestCopy = request.encode();
    
    // send request
    std::string reqStr(requestCopy.begin(), requestCopy.end());
    if (send(sockfd, reqStr.c_str(), reqStr.size(), 0) == -1) {
      perror("send");
      return 4;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////
    
    // receive response
    bool isEnd = false;
    std::string input;
    char buf[20] = { 0 };
    std::stringstream ssOverall;
    std::stringstream ssIteration;
    const string endingStr = "\r\n\r\n";
    unsigned int endingCount = 0;
    HttpResponse response;
    
    while(!isEnd) {
      memset(buf, '\0', sizeof(buf));
      
      std::cout <<"send: ";
      std::cin >> input;
      if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
	perror("send");
	return 4;
      }
      
      if (recv(sockfd, buf, 20, 0) == -1) {
	perror("recv");
	return 5;
      }
      
      ssOverall << buf;
      ssIteration << buf;
      string currString = ssIteration.str();
      
      // Test for end reached
      for (unsigned int i = 0; i < currString.length(); i++) {
	if (currString[i] == endingStr[endingCount])
	  endingCount++;
	else
	  endingCount = 0;
	if (endingCount == 4) {
	  std::string totalRespString = ssOverall.str();
	  vector<uint8_t> decoded(totalRespString.begin(), totalRespString.end());
	  response = HttpResponse::decode((ByteBlob)decoded);
	  ssOverall.str("");
	  endingCount = 0;
	  isEnd = true;
	}
      }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    
    // get filename
    std::string path = request.getUrl();
    std::stringstream ss(path);
    std::string token, filename;
    
    while(std::getline(ss, token, '/')) {
      filename = token;
    }
    
    if (filename.empty())
      filename = "index.html";
    
    // parse to distinguish success or failure
    HttpStatus status = response.getStatus();
    if (status == "OK_200")
      std::cout << "200 OK\nSaving file to " << filename << "\n";
    else if (status == "BR_400")
      std::cout << "400 Bad Request\n";
    else /* (status == "NF_404") */
      std::cout << "404 Not Found\n";
    
    // exit if unsuccessful
    if(status != "OK_200")
      exit(1);
    
    // save to current directory
    std::ofstream os(filename);
    if (!os) {
      std::cerr<<"Error writing to ..."<<std::endl;
    }
    else {
      os << response.getData();
    }
    // free stuff, close socket
    close(sockfd);
    
    return 0;
  }
}
