#ifndef RCCLIENT_H_INCLUDED
#define RCCLIENT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <fstream>
#include <iostream>

using namespace std;

#define PORT "3490"
#define LOG_FILE "/home/chaginsergey/Downloads/Server_files/log_file.log"


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

    void server_handler(int sockfd);

    void showHomeDirContent(int sockfd);
    void showCurrDirName(int sockfd);
    void changeDirTo(int sockfd);
    void showCurrDirContent(int sockfd);
    void showFileInfo(int sockfd);
    void deleteFile(int sockfd);
    void uploadFileToServer(int sockfd);
    void downloadFileFromServer(int sockfd);

    string generaneRandFileName();
    void readFileAndSend(int sockfd, string file_to_upload);
    string readFile(string file_name);

    void sendData(int sockfd, string& data);
    string recvData(int sockfd);
    void sendErrorMsg(int sockfd, string er_msg);

    void* get_in_addr(struct sockaddr *sa);
    ssize_t sendall(int sockfd, const char *buf, size_t *len);


};
#endif // RCCLIENT_H_INCLUDED
