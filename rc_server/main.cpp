#include <iostream>
#include <fstream>
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
#include <fcntl.h>

#define PORT "3490"
#define BACKLOG 10

#define LOG_FILE "/home/chaginsergey/Downloads/Server_files/log_file.log"
#define SERV_DIR "/home/chaginsergey/Downloads/Server_files/"


using namespace std;

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int sendall(int s, const char *buf, int *len);
void log_message(string message);
void recv_file_and_save(int new_fd);
void save_file(string file_content);
int validationInput();
void send_msg_to_exit(int sockfd);
void do_some_work(int sockfd);
void show_home_dir(int sockfd);
void navigate(int sockfd);
void send_command(int sockfd, int cmnd);
void recv_file_and_show(int sockfd);

enum COMMANDS
{
    EXIT = 111,
    SHOW_DIR,
    GET_FILE,
};


void client_handler(int sockfd)
{
    while(1)
    {
        printf("Enter command> ");
        int comnd = validationInput();

        switch(comnd)
        {
            case 1:
            {
                printf("Exit msg\n"); send_msg_to_exit(sockfd); exit(0); break;
            }
            case 2:
            {
                printf("Show dir msg\n"); show_home_dir(sockfd); break;
            }
            case 3:
            {
                printf("Recv file msg\n"); recv_file_and_save(sockfd); break;
            }
            case 4:
            {
                printf("Do some work msg\n"); do_some_work(sockfd); break;
            }
            case 5:
            {
                printf("Getting a file from remote pc\n"); navigate(sockfd); break;
            }
            default: exit(0);

        }

    }

}

void navigate(int sockfd)
{
    send_command(sockfd, 5);

    recv_file_and_show(sockfd);

//    recv_file_and_save(sockfd);

}


void send_command(int sockfd, int cmnd)
{
    int bytesSend = send(sockfd, reinterpret_cast<char*>(&cmnd), sizeof(int), 0);
    if (bytesSend == -1)
    {
        perror("send");
    }

    if (bytesSend == 0)
    {
        perror("send 0");
    }
}

void send_msg_to_exit(int sockfd)
{
    send_command(sockfd, 1);

}

void show_home_dir(int sockfd)
{
    send_command(sockfd, 2);

    recv_file_and_save(sockfd);
}


void do_some_work(int sockfd)
{
    send_command(sockfd, 4);
}


string generate_file_name()
{
    string file_name = SERV_DIR;

    time_t now = time(0);
    char* dt = ctime(&now);

    file_name.append(dt);

    return file_name;

}

void save_file(char* file_content, int msg_size)
{
    string file_name = generate_file_name();

    ofstream outf;
    outf.open(file_name);
    if (!outf)
	{
		fprintf (stderr, "Can't open: %s\n", file_name);
		return;
	}

	outf<<file_content;

	outf.close();
}

void recv_file_and_save(int sockfd)
{
    send_command(sockfd, 3);

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

    string file_name = generate_file_name();

    string data_file;

    data_file.assign(buff, buff+msg_size);

    ofstream ofs;
    ofs.open(file_name);

    ofs<<data_file;

    ofs.close();
}

void recv_file_and_show(int sockfd)
{
    send_command(sockfd, 3);

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

    cout<<string(buff, msg_size)<<endl;
}

int main(int argc, char* argv[])
{
    int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char addr[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

    freeaddrinfo(servinfo);

    if (p == NULL)
    {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

    sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

    pid_t status, childpid;
    int exit_status;

    while(1)
    {
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), addr, sizeof addr);
		printf("server: got connection from %s\n", addr);

		//TODO : authorization
		//if not - exit


        client_handler(new_fd);

        //handling one client so far
        /*
        status = fork();

        if(status == -1)
        {
            perror("Fork error");
            return -1;
        }

		if (status == 0) // this is the child process
		{
			close(sockfd); // child doesn't need the listener

			client_handler(new_fd); //handle client's requests

            close(new_fd);

            return 0;

        }
        */
        childpid = wait(&exit_status);

        if(WIFEXITED(exit_status))
        {
            printf("Process with PID=%d has exited with code=%d\n", childpid, WEXITSTATUS(exit_status));

        }

        if (WIFSIGNALED (exit_status))
        {
            printf ("Process with PID=%d has exited with signal.\n", childpid);
        }

    }


    return EXIT_SUCCESS;

}



void sigchld_handler(int s)
{
	(void)s;

	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int sendall(int s, const char *buf, int *len)
{
    int total = 0; // how much bytes was sent
    int bytesleft = *len; // how much bytes left
    int n;

    while(total < *len)
    {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total;// really sent
    return n==-1?-1:0; // -1 error, 0 success
}

void log_message(string message)
{
    ofstream outf;
    outf.open(LOG_FILE, ios::app);
    if (!outf)
	{
		fprintf (stderr, "Can't open: %s\n", LOG_FILE);
		exit(1);
	}

	outf<<message<<endl;

	outf.close();
}

int validationInput()
{
    int val;
    while (true)
    {
        std::cin >> val;
        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(32767, '\n');
            std::cerr << "Ошибка ввода! Попробуйте ещё раз: ";
            continue;
        }
        std::cin.ignore(32767, '\n');
        return val;
    }
}
