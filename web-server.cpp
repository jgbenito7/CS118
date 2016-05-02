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

#include <thread>

#include "http.h"

typedef std::string string;

HttpResponse processRequest(HttpRequest r, string dir){

	HttpResponse resp;
	string url;
	string error;

	//Check if this is a relative dir
	if(dir.front() == '.'){
		dir.erase(0,2);
	}

	//check if this is root
	if(dir.back()=='/' && dir.size()==1){
		url = string("/") + r.getUrl();
		cout << url << endl;

	}else if(dir.back()=='/'){ //check for trailing slash
		url = dir + r.getUrl();
		cout << url << endl;
	}else{
		url = dir + "/" + r.getUrl();
		cout << url << endl;
	}
	std::ifstream in(url.c_str());
	/*if(() >> error){
			cout << "Success";
	}else{
			cout << "Error code: " << strerror(errno);
	}*/

	//cout << "Url: " << url << endl;



	ByteBlob contents((std::istreambuf_iterator<char>(in)),
	    std::istreambuf_iterator<char>());
	//cout << contents << '\n';
	if(contents.empty()){
		resp.setStatus(HttpResponse::NF_404);
		return resp;
	}
	resp.setHeader("content-language", "en");
	resp.setHeader("content-type", "text/html; charset=UTF-8");
	// resp.setHeader("content-length", "text/html; charset=UTF-8");
	// resp.setHeader("Accept", "text/html,application/xhtml+xml");
	//resp.setHeader("Accept-Language", "en-us,en;q=0.5");
	resp.setData(contents);
	resp.setStatus(HttpResponse::OK_200);
	//in.close();

	return resp;
}

void receiveRequest(int clientSockfd, string dir){


	// read/write data from/into the connection
	bool isEnd = false;
	char buf[1000] = { 0 };
	stringstream ssOverall;
	stringstream ssIteration;
	const string endingStr = "\r\n\r\n";
	unsigned int endingCount = 0;

	while (!isEnd) {
		memset(buf, '\0', sizeof(buf));

		if (recv(clientSockfd, buf, 1000, 0) == -1) {
			perror("recv");
			return;// 5;
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
				// cout << "--------totalReqString--------" << endl << totalReqString << endl;
				vector<uint8_t> decoded(totalReqString.begin(), totalReqString.end());
				HttpRequest req = HttpRequest::decode((ByteBlob)decoded);
				HttpResponse resp = processRequest(req, dir); //Process the request object

				ByteBlob respBB = resp.encode();
				uint8_t* respBytes = &respBB[0];
		  	int respBytesSize = sizeof(uint8_t) * respBB.size();

				cout << "Num bytes being sent total: " << respBB.size() << endl;
				// cout << "Num bytes being sent, data: " << resp.getData().size() << endl;
				std::ofstream os("asdfasdfasdf.jpg");
				if (!os) {
					std::cerr<<"Error writing to ..."<<std::endl;
				}
				else {
					HttpResponse decodedResp = HttpResponse::decode(respBB);
					// ByteBlob data = resp.getData();
					ByteBlob data = decodedResp.getData();
					for(ByteBlob::iterator x=data.begin(); x<data.end(); x++){
						os << *x;
					}
					os.close();
				}
				if (send(clientSockfd, respBytes, respBytesSize, 0) == -1) {
					perror("send");
					return;// 6;
				}

				//ssOverall.str(""); doesn't 	matter, we're closing connection
				//endingCount = 0;
				isEnd = true;
				break;
			}
		}
	}
	close(clientSockfd);
	cout << "Server closing" << endl;;
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

	while(true){
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

		thread(receiveRequest, clientSockfd, file_dir).detach();
	}
	return 0;
}
