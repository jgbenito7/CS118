#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "showip.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "http.h"

typedef std::string string;

HttpResponse processRequest(HttpRequest r){
	HttpResponse resp;
	string url = r.getUrl();

	std::ifstream in(url.c_str());
	std::string contents((std::istreambuf_iterator<char>(in)),
	    std::istreambuf_iterator<char>());
	cout << contents << '\n';
	if(contents.empty()){
		resp.setStatus(HttpResponse::NF_404);
		return resp;
	}
	resp.setData(contents);
	//in.close();

	 return resp;
}

int
main(int argc, char* argv[])
{
	// create a socket using TCP IP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	char* hostN = argv[1];
	char* portN = argv[2];
	string file_dir = argv[3];

	int portNumInt = std::stoi(portN);

	// allow others to reuse the address
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return 1;
	}

	//TODO HERE
	//Convert hostN to ip address
	string hostIP = getIP(hostN);
	cout << hostIP << endl;
	// bind address to socket
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portNumInt);     // short, network byte order
	addr.sin_addr.s_addr = inet_addr(hostIP.c_str());
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 2;
	}

	// set socket to listen status
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		return 3;
	}

	// accept a new connection
	struct sockaddr_in clientAddr;
	socklen_t clientAddrSize = sizeof(clientAddr);
	int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

	if (clientSockfd == -1) {
		perror("accept");
		return 4;
	}

	char ipstr[INET_ADDRSTRLEN] = { '\0' };
	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	std::cout << "Accept a connection from: " << ipstr << ":" <<
		ntohs(clientAddr.sin_port) << std::endl;

	// read/write data from/into the connection
	bool isEnd = false;
	char buf[20] = { 0 };
	stringstream ssOverall;
	stringstream ssIteration;
	const string endingStr = "\r\n\r\n";
	unsigned int endingCount = 0;
	while (!isEnd) {
		memset(buf, '\0', sizeof(buf));

		if (recv(clientSockfd, buf, 20, 0) == -1) {
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
				string totalReqString = ssOverall.str();
				vector<uint8_t> decoded(totalReqString.begin(), totalReqString.end());
				HttpRequest req = HttpRequest::decode((ByteBlob)decoded);
				processRequest(req);
				ssOverall.str("");
				endingCount = 0;
			}
		}
		//std::cout << buf << std::endl;


		if (send(clientSockfd, buf, 20, 0) == -1) {
			perror("send");
			return 6;
		}
	}

	close(clientSockfd);

	return 0;
}
