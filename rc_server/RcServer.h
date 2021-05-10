#ifndef RCSERVER_H_INCLUDED
#define RCSERVER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>

#include <iostream>
#include <fstream>

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
	char addr[INET6_ADDRSTRLEN];
	int rv;

    const char* help_str = "case 1: Exit\n"
                        "case 2: Home dir content\n"
                        "case 3: Current dir name\n"
                        "case 4: Channge dir to\n"
                        "case 5: Current dir content\n"
                        "case 6: File info\n"
                        "case 7: Delete file\n"
                        "case 8: Download file\n"
                        "case 9: Upload file\n"
                        "case 0: Print this help\n";

public:

    RcServer();
    ~RcServer();

    void start();
    void stop();
//    void handleNewConn();
//    void handleThisConn();

    void client_handler(int sockfd);

protected:

    void sendCommand(int sockfd, int cmnd);

    void send_msg_to_exit(int sockfd); //1
    void show_home_dir_content(int sockfd); //2
    void show_cur_dir_name(int sockfd); //3
    void change_dir_to(int sockfd); //4
    void show_cur_dir_content(int sockfd); //5
    void show_file_detail_info(int sockfd); //6
    void delete_file(int sockfd); //7
    void download_file(int sockfd); //8
    void upload_file(int sockfd); //9

    string recvData(int sockfd);
    void sendData(int sockfd, string data);

    void recvFileAndShow(int sockfd);
    void recvFileAndSave(int sockfd, string file_name);

    void uploadFileToClient(int sockfd);

    void sendStringData(int sockfd);
    void sendStringData(int sockfd, string& file_name); //TODO: rewrite

    string read_file(string file_name);
    string generate_file_name();

    void log_message(string message, ssize_t val);
    int validationInput();

    void* get_in_addr(struct sockaddr*);
    ssize_t sendall(int sockfd, const char *buf, size_t *len);

    friend void sigchld_handler(int s);

};

#endif // RCSERVER_H_INCLUDED
