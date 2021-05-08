#include <fstream>
#include <iostream>
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
#include <dirent.h>

#include <string.h>
#include <sys/stat.h>

using namespace std;

enum COMMANDS
{
    EXIT = 111,
    SHOW_DIR,
    GET_FILE,
};

#define PORT "3490"

#define LOG_FILE "/home/chaginsergey/Downloads/Server_files/log_file.log"
#define SERV_DIR "/home/chaginsergey/Downloads/Server_files/"
#define MSG_FILE "/home/chaginsergey/Downloads/Enron_Complex.jpg"

void read_file_and_send(int sockfd, string file_name);
string read_file(string file_name);
void send_info_to_server(int sockfd, string data_to_send);
string recv_info_from_server(int sockfd);
int sendall(int sockfd, const char *buf, int *len);
void *get_in_addr(struct sockaddr *sa);


void show_home_dir(int sockfd)
{
    printf("show_home_dir\n");

    DIR *dir;
    struct dirent *entry;

    dir = opendir (getenv("HOME"));

    if (dir == NULL)
    {
        fprintf (stderr, "opendir() error\n");
        return;
    }

    string data_to_send = "Catalogues info:\n";

    while ((entry = readdir (dir)) != NULL)
    {
        string temp = entry->d_name;
        if(temp[0] == '.') continue;

        data_to_send.append(entry->d_name).append("\n");
    }

    cout<<data_to_send<<endl;

    send_info_to_server(sockfd, data_to_send);
}

void show_cur_dir_name(int sockfd)
{
    int buf_size = 1024;
    char *buf;

    buf = getcwd (NULL, buf_size);

    if (buf == NULL)
    {
        fprintf (stderr, "getcwd() error\n");
        return;
    }

    send_info_to_server(sockfd, buf);

    free (buf);

}

void change_dir_to(int sockfd)
{
    string data = recv_info_from_server(sockfd);

    if (chdir(data.c_str()) == -1)
    {
        fprintf (stderr, "chdir() error\n");
        return;
    }
}

void show_cur_dir_content(int sockfd)
{
    DIR *dir;
    struct dirent * entry;

    int buf_size = 1024;
    char *buf;

    string data_to_send;

    buf = getcwd (NULL, buf_size);

    if (buf == NULL)
    {
        fprintf (stderr, "getcwd() error\n");
        return;
    }

    dir = opendir(buf);

    if (dir == NULL)
    {
        fprintf (stderr, "opendir() error\n");
        return;
    }

    while ((entry = readdir (dir)) != NULL)
    {
        string temp = entry->d_name;
        if(temp[0] == '.') continue;
        data_to_send.append(temp).append("\n");
    }

    send_info_to_server(sockfd, data_to_send);

    closedir (dir);
    free (buf);

}

void show_file_detail_info(int sockfd)
{
    struct stat st;

    string data = recv_info_from_server(sockfd);

    if (stat (data.c_str(), &st) == -1)
    {
        fprintf (stderr, "stat() error\n");
        return;
    }

    string data_to_send;
    data_to_send.append("FILE:\t\t").append(data).append("\n")
    .append("UID:\t\t").append( to_string((int)st.st_uid)).append("\n")
    .append("GID:\t\t").append(to_string((int) st.st_gid)).append("\n")
    .append("SIZE:\t\t").append(to_string((long int) st.st_size)).append(" bytes\n")
    .append("AT:\t\t").append(ctime (&st.st_atime))
    .append("MT:\t\t").append(ctime (&st.st_mtime)).append("\n");

    send_info_to_server(sockfd, data_to_send);

}

void delete_file(int sockfd)
{
    string data = recv_info_from_server(sockfd);

    if (unlink (data.c_str()) == -1)
    {
        fprintf (stderr, "Cannot unlink file (%s)\n", data.c_str());
        return;
    }

}

void server_handler(int sockfd)
{
    int cmnd=0;

    while(1)
    {
        int bytesRecv = recv(sockfd, &cmnd, sizeof(int), 0);

        switch(cmnd)
        {
            case 1:
            {
                printf("Exit programm\n"); exit(0); break;
            }
            case 2:
            {
                show_home_dir(sockfd); break;
            }
            case 3:
            {
                show_cur_dir_name(sockfd); break;
            }
            case 4:
            {
                change_dir_to(sockfd); break;
            }
            case 5:
            {
                show_cur_dir_content(sockfd); break;
            }
            case 6:
            {
                show_file_detail_info(sockfd); break;
            }
            case 7:
            {
                delete_file(sockfd); break;
            }
            default: break;

        }

    }

}

string recv_info_from_server(int sockfd)
{
    int msg_size;

    int bytesRecv = recv(sockfd, &msg_size, sizeof(int), 0);

    if(bytesRecv == -1)
    {
        perror("recv -1");
        exit(EXIT_FAILURE);
    }
    else if(bytesRecv == 0)
    {
        perror("recv 0");
        exit(EXIT_FAILURE);
    }

    char* buff = new char[msg_size+1];

    bytesRecv = recv(sockfd, buff, msg_size, 0);

    if(bytesRecv == -1)
    {
        perror("recv -1");
        exit(EXIT_FAILURE);
    }
    else if(bytesRecv == 0)
    {
        perror("recv 0");
        exit(EXIT_FAILURE);
    }

    return string(buff, msg_size);

}

string read_file(string file_name)
{
    string o_file;

    ifstream fin(file_name, ios::binary);

    if(!fin.is_open())
    {
        printf("Cannot read file");
        return "";
    }

    size_t file_size = fin.seekg(0, ios::end).tellg();
    fin.seekg(0);
    char * buffer = new char[file_size];

    fin.read(buffer, file_size);
    fin.close();

    o_file.assign(buffer, buffer + file_size);

    return o_file;

//    ifstream inf;
//    inf.open(file_name);
//    if (!inf)
//	{
//		fprintf (stderr, "Can't open: %s\n", file_name);
//		exit(1);
//	}
//
//	string line;
//	string content;
//
//	while (inf)
//	{
//        getline(inf, line);
//        content+=line;
//	}
//
//    inf.close();

}

void read_file_and_send(int sockfd, string file_name)
{
    string data_to_send = read_file(MSG_FILE);

    int data_size = data_to_send.size();

    int bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(int), 0);

    bytesSend = sendall(sockfd, data_to_send.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }
}

void send_info_to_server(int sockfd, string data_to_send)
{
    int data_size = data_to_send.size();

    int bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(int), 0);

    bytesSend = sendall(sockfd, data_to_send.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }

}

int main(int argc, char* argv[])
{
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0)
    {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);

    server_handler(sockfd);

    close(sockfd);

    return 0;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int sendall(int sockfd, const char *buf, int *len)
{
    int total = 0;
    int bytesleft = *len;
    int num;

    while(total < *len)
    {
        num = send(sockfd, buf+total, bytesleft, 0);
        if (num == -1) { break; }
        total += num;
        bytesleft -= num;
    }

    *len = total;
    return num == -1? -1 : 0;
}


