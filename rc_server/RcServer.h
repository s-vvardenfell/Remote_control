#ifndef RCSERVER_H_INCLUDED
#define RCSERVER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <fstream>
#include <string>
#include <iostream>

#define PORT "3490"
#define BACKLOG 10

#define LOG_FILE "/home/chaginsergey/Downloads/Server_files/log_file.log"
#define SERV_DIR "/home/chaginsergey/Downloads/Server_files/"

using namespace std;

class RcServer
{
private:
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

public:

    RcServer();
    ~RcServer();

    void start();
    void stop();
    void handleNewConn();
    void handleThisConn();
    void* get_in_addr(struct sockaddr*);

    void err_handler(); //not impl yet

    void recv_file_and_save();
    void save_file(string);

    friend void sigchld_handler(int s);

};

#endif // RCSERVER_H_INCLUDED
