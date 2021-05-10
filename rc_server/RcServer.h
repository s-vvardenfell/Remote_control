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

public:

    RcServer();
    ~RcServer();

    void start();
    void stop();
//    void handleNewConn();
//    void handleThisConn();

    void client_handler(int sockfd);

protected:

    void send_command(int sockfd, int cmnd);

    void send_msg_to_exit(int sockfd); //1
    void show_home_dir_content(int sockfd); //2
    void show_cur_dir_name(int sockfd); //3
    void change_dir_to(int sockfd); //4
    void show_cur_dir_content(int sockfd); //5
    void show_file_detail_info(int sockfd); //6
    void delete_file(int sockfd); //7
    void download_file(int sockfd); //8
    void upload_file(int sockfd); //9

    void send_string_to_client(int sockfd);
    void send_string_to_client(int sockfd, string& file_name);
    void send_file_to_client(int sockfd);
    string read_file(string file_name);
    string generate_file_name();
    void recv_file_and_save(int sockfd, string file_name);
    void recv_file_and_show(int sockfd);

    const char* help_str = "case 1: Exit programm\n"
                        "case 2: show_home_dir_content\n"
                        "case 3: show_cur_dir_name\n"
                        "case 4: change_dir_to\n"
                        "case 5: show_cur_dir_content\n"
                        "case 6: show_file_detail_info\n"
                        "case 7: delete_file\n"
                        "case 8: download_file\n"
                        "case 9: upload_file\n";


    void print_err_and_exit(const char* msg, ssize_t val, const char* func);
    void log_message(string message, ssize_t val);
    int validationInput();

    void* get_in_addr(struct sockaddr*);
    ssize_t sendall(int sockfd, const char *buf, size_t *len);

    friend void sigchld_handler(int s);

};

#endif // RCSERVER_H_INCLUDED
