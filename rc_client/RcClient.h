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

    void show_home_dir_content(int sockfd);
    void show_cur_dir_name(int sockfd);
    void change_dir_to(int sockfd);
    void show_cur_dir_content(int sockfd);
    void show_file_detail_info(int sockfd);
    void delete_file(int sockfd);
    void upload_file(int sockfd);
    void download_file(int sockfd);



    void downloadFileFromServer(int sockfd);
    string generate_file_name();
    void read_file_and_send(int sockfd, string file_to_upload);
    string read_file(string file_name);

    void sendData(int sockfd, string data);
    string recvData(int sockfd);

    void* get_in_addr(struct sockaddr *sa);
    ssize_t sendall(int sockfd, const char *buf, size_t *len);


};
#endif // RCCLIENT_H_INCLUDED
