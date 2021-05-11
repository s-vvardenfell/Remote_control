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

    void clientHandler(int sockfd);

protected:

    void sendCommand(int sockfd, int cmnd);

    void sendMsgToExit(int sockfd); //1
    void showHomeDirContent(int sockfd); //2
    void showCurrDirName(int sockfd); //3
    void changeDirTo(int sockfd); //4
    void showCurrDirContent(int sockfd); //5
    void showFileInfo(int sockfd); //6
    void deleteFile(int sockfd); //7
    void downloadFile(int sockfd); //8
    void uploadFile(int sockfd); //9

    string recvData(int sockfd);
    void sendData(int sockfd, string& data);
    void sendErrorMsg(int sockfd, string& data);

    void recvFileAndShow(int sockfd);
    void recvFileAndSave(int sockfd, string file_name);

    void uploadFileToClient(int sockfd);

    void sendStringData(int sockfd);
    void sendStringData(int sockfd, string& file_name); //TODO: rewrite

    string readFile(string file_name);
    string generaneRandFileName();

    void logMsg(string message, ssize_t val);
    int validationInput();

    void* get_in_addr(struct sockaddr*);
    ssize_t sendall(int sockfd, const char *buf, size_t *len);

    friend void sigchld_handler(int s);

};

#endif // RCSERVER_H_INCLUDED
