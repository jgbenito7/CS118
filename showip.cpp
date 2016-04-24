#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>
#include <iostream>

using namespace std;

string getIP(char* address){
  struct addrinfo hints;
  struct addrinfo* res;

  // prepare hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // get address
  int status = 0;
  if ((status = getaddrinfo(address, "80", &hints, &res)) != 0) {
    cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    return "error";
  }

  // convert the IP to a string and print it:
  char ipstr[INET_ADDRSTRLEN] = {'\0'};

  for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
    // convert address to IPv4 address
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

    inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
    //std::cout << "  " << ipstr << std::endl;
    // std::cout << "  " << ipstr << ":" << ntohs(ipv4->sin_port) << std::endl;

  }

  freeaddrinfo(res); // free the linked list

  return ipstr;
}


int main()
{
  cout << getIP("localhost") << endl;

  return 0;
}
