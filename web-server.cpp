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
#include <sstream>

int
main(int argc, char* argv[])
{
	// create a socket using TCP IP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	char* hostN = argv[1];
	char* portN = argv[2];
	std::string file_dir = argv[3];

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
	std::stringstream ssOverall;
	std::stringstream ssIteration;
	std::string endingStr = "\r\n\r\n";
	unsigned int endingCount = 0;
	while (!isEnd) {
		memset(buf, '\0', sizeof(buf));

		if (recv(clientSockfd, buf, 20, 0) == -1) {
			perror("recv");
			return 5;
		}

		ssOverall << buf;
		ssIteration << buf;
		std::string currString = ssIteration.str();

		for(unsigned int i = 0; i < currString.length(); i++){
			if(currString[i] == endingStr[endingCount])
				endingCount++;
			else
				endingCount = 0;
			if(endingCount == 4){
				HttpRequest req = HttpMessage::decode((ByteBlob)(ssOverall.str().begin(), ssOverall.str().end));
			}
		}
		std::cout << buf << std::endl;


		if (send(clientSockfd, buf, 20, 0) == -1) {
			perror("send");
			return 6;
		}

		if (ss.str() == "close\n")
			break;

		ss.str("");
	}

	close(clientSockfd);

	return 0;
}
