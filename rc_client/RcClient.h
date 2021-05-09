#ifndef RCCLIENT_H_INCLUDED
#define RCCLIENT_H_INCLUDED

#include <fstream>
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

#define PORT "3490"


class RcClient
{

private:
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];


public:

    RcClient();
    ~RcClient();

    void start();
    void stop();

    void* get_in_addr(struct sockaddr *sa);
    int sendall(int sockfd, const char *buf, int *len);
    string read_file(string file_name);
    void read_file_and_send(string file_name);


};
#endif // RCCLIENT_H_INCLUDED
