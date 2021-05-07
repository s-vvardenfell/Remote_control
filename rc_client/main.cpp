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
int sendall(int sockfd, const char *buf, int *len);
void *get_in_addr(struct sockaddr *sa);
void skan_dir_and_send(int sockfd);
void navigate(int sockfd);

void show_home_dir()
{
    printf("show_home_dir\n");

    DIR *dir;
    struct dirent *entry;

    dir = opendir (getenv("HOME")); //получаем имя домашнего каталога/директории через переменную окружения

    if (dir == NULL)
    {
        fprintf (stderr, "opendir() error\n");
        return;
    }

    string data_to_send = "Catalogues info:\n";

    while ((entry = readdir (dir)) != NULL) //получаем содержимое каталога/директории
    {
        string temp = entry->d_name;
        if(temp[0] == '.') continue;

        data_to_send.append(entry->d_name).append("\n");
//        printf ("%s\n", entry->d_name;
    }

    cout<<data_to_send<<endl;
}

void show_cur_dir_name()
{
    printf("show_cur_dir\n");

    int buf_size = 1024;
    char * buf;
    if (buf == NULL)
    {
        fprintf (stderr, "buf is NULL\n");
        return;
    }

    buf = getcwd (NULL, buf_size);

    if (buf == NULL)
    {
        fprintf (stderr, "getcwd() error\n");
        return;
    }

    printf ("Current directory: %s\n", buf);
    free (buf);

}

void change_dir_to()
{
    printf("change_dir_to\n");

    string new_dir;
    getline(cin, new_dir);

    if (chdir (new_dir.c_str()) == -1)
    {
        fprintf (stderr, "chdir() error\n");
        return;
    }
}

void show_cur_dir_content()
{
    DIR * dir;
    struct dirent * entry;
    //
    int buf_size = 1024;
    char * buf;
    if (buf == NULL)
    {
        fprintf (stderr, "buf is NULL\n");
        return;
    }

    buf = getcwd (NULL, buf_size);

    if (buf == NULL)
    {
        fprintf (stderr, "getcwd() error\n");
        return;
    }

    printf ("Current directory: %s\n", buf);
    printf ("Basename: %s\n", basename (buf));

    dir = opendir(buf);

    if (dir == NULL)
    {
        fprintf (stderr, "opendir() error\n");
        return;
    }

    printf ("Current directory contents: %s\n", buf);

    while ((entry = readdir (dir)) != NULL)
        printf ("%s\n", entry->d_name);

    closedir (dir);
    free (buf);

}

void show_file_detail_info()
{
    struct stat st;

    string new_dir;
    getline(cin, new_dir);

    if (stat (new_dir.c_str(), &st) == -1)
    {
        fprintf (stderr, "stat() error\n");
        return;
    }

    printf ("FILE:\t\t%s\n", new_dir.c_str());
    printf ("UID:\t\t%d\n", (int) st.st_uid);
    printf ("GID:\t\t%d\n", (int) st.st_gid);
    printf ("SIZE:\t\t%ld\n", (long int) st.st_size);
    printf ("AT:\t\t%s", ctime (&st.st_atime));
    printf ("MT:\t\t%s", ctime (&st.st_mtime));

}

void delete_file()
{
    string file_to_delete;
    getline(cin, file_to_delete);

    if (unlink (file_to_delete.c_str()) == -1)
    {
        fprintf (stderr, "Cannot unlink file (%s)\n", file_to_delete.c_str());
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
                printf("Exit msg\n"); exit(0);
            }
            case 2:
            {
                printf("Show dir msg\n"); skan_dir_and_send(sockfd); break;
            }
            case 3:
            {
                printf("Send file msg\n"); read_file_and_send(sockfd, MSG_FILE); break;
            }
            case 4:
            {
                printf("Doing some work\n"); break;
            }
            case 5:
            {
                printf("Navigating from remote pc\n"); navigate(sockfd); break;
            }
            default : exit(0);
        }

    }

}

void navigate(int sockfd)
{
    skan_dir_and_send(sockfd);

}

void skan_dir_and_send(int sockfd)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir (getenv("HOME")); //получаем имя домашнего каталога/директории через переменную окружения

    if (dir == NULL)
    {
        fprintf (stderr, "opendir() error\n");
        return;
    }

    string data_to_send = "Catalogues info:\n";

    while ((entry = readdir (dir)) != NULL) //получаем содержимое каталога/директории
    {
        string temp = entry->d_name;
        if(temp[0] == '.') continue;

        data_to_send.append(entry->d_name).append("\n");
//        printf ("%s\n", entry->d_name;
    }

    if(closedir (dir) == -1)
    {
        fprintf (stderr, "closedir() error\n");
        return;
    }

    int data_size = data_to_send.size();

    int bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(int), 0);
    bytesSend = sendall(sockfd, data_to_send.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }

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


