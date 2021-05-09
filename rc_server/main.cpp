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

enum COMMANDS
{
    EXIT = 111,
    SHOW_HOME_DIR,
    SHOW_CURR_DIR_NAME,
    CHANGE_DIR,
    SHOW_CUR_DIR_CONT,
    SHOW_FILE_DETAIL_INFO,
    DELETE_FILE,
    DOWNLOAD_FILE,
    UPLOAD_FILE
};


void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
ssize_t sendall(int s, const char *buf, size_t *len);
int validationInput();

void log_message(string message, int val);

void recv_file_and_save(int new_fd, string file_name);
void recv_file_and_show(int new_fd);

void send_command(int sockfd, int cmnd);
void send_string_to_client(int sockfd);
void send_string_to_client(int sockfd, string& file_name);
void send_file_to_client(int sockfd);

string read_file(string file_name);
void save_file(string file_name); //don't have impl yet

void send_msg_to_exit(int sockfd)
{
    send_command(sockfd, 1);
}

void show_home_dir(int sockfd)
{
    send_command(sockfd, 2);
    recv_file_and_show(sockfd);
}

void show_cur_dir_name(int sockfd)
{
    send_command(sockfd, 3);
    recv_file_and_show(sockfd);
}

void change_dir_to(int sockfd)
{
    send_command(sockfd, 4);
    send_string_to_client(sockfd);
}

void show_cur_dir_content(int sockfd)
{
    send_command(sockfd, 5);
    recv_file_and_show(sockfd);
}

void show_file_detail_info(int sockfd)
{
    send_command(sockfd, 6);
    send_string_to_client(sockfd);
    recv_file_and_show(sockfd);
}

void delete_file(int sockfd)
{
    send_command(sockfd, 7);
    send_string_to_client(sockfd);
}

void download_file(int sockfd)
{
    send_command(sockfd, 8);
    string file_name = SERV_DIR;
    send_string_to_client(sockfd, file_name);
    recv_file_and_save(sockfd, file_name);
}

void upload_file(int sockfd)
{
    send_command(sockfd, 9);
    send_file_to_client(sockfd);
}


const char* help_str = "case 1: Exit programm\n"
                        "case 2: show_home_dir\n"
                        "case 3: show_cur_dir_name\n"
                        "case 4: change_dir_to\n"
                        "case 5: show_cur_dir_content\n"
                        "case 6: show_file_detail_info\n"
                        "case 7: delete_file\n"
                        "case 8: download_file\n"
                        "case 9: upload_file\n";


void client_handler(int sockfd)
{
    while(1)
    {
        printf("%s\n", help_str);
        show_cur_dir_name(sockfd);
        printf("$");
        int cmnd = validationInput();

        switch(cmnd)
        {
            case 1:
            {
                printf("Exiting programm\n"); send_msg_to_exit(sockfd); exit(0);
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
            case 8:
            {
                download_file(sockfd); break;
            }
            case 9:
            {
                upload_file(sockfd); break;
            }
            default: break;

        }

    }

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

void send_string_to_client(int sockfd)
{
    string msg;
    getline(cin, msg);

    size_t data_size = msg.size();

    ssize_t bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(size_t), 0);

    bytesSend = sendall(sockfd, msg.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }
}

void send_string_to_client(int sockfd, string& file_name)
{
    string msg;
    getline(cin, msg);

    file_name += msg;

    size_t data_size = msg.size();

    ssize_t bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(size_t), 0);

    bytesSend = sendall(sockfd, msg.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }
}

void send_file_to_client(int sockfd)
{
    string file_name, file_data, file_path = SERV_DIR;
    getline(cin, file_name);

    file_path+=file_name;

    //отправим имя сначала
    size_t data_size = file_name.size();

    ssize_t bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(size_t), 0);

    bytesSend = sendall(sockfd, file_name.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }

    //отправим данные
    file_data = read_file(file_path);

    data_size = file_data.size();

    bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(size_t), 0);

    bytesSend = sendall(sockfd, file_data.c_str(), &data_size);

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
}

string generate_file_name()
{
    string file_name = SERV_DIR;

    time_t now = time(0);
    char* dt = ctime(&now);

    file_name.append(dt);
    file_name.erase(file_name.size()-1, 1);

    return file_name;
}

void recv_file_and_save(int sockfd, string file_name)
{
    size_t msg_size;

    ssize_t bytesRecv = recv(sockfd, &msg_size, sizeof(size_t), 0);

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

//    string file_name = generate_file_name();

    string data_file;

    data_file.assign(buff, buff+msg_size);

    ofstream ofs;
    ofs.open(file_name);

    if(!ofs.is_open())
    {
        printf("Cannot write file");
        return ;
    }

    ofs<<data_file;
//    ofs.write(buff, msg_size);

    ofs.close();

}

void recv_file_and_show(int sockfd)
{
    size_t msg_size;

    ssize_t bytesRecv = recv(sockfd, &msg_size, sizeof(size_t), 0);

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

    string info_to_show(buff, msg_size);
    info_to_show.erase(info_to_show.size(), 1);
    cout<<info_to_show;
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

ssize_t sendall(int s, const char *buf, size_t *len)
{
    size_t total = 0; // how much bytes was sent
    size_t bytesleft = *len; // how much bytes left
    ssize_t n;

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

void log_message(string message, ssize_t val)
{
    ofstream outf;
    outf.open(LOG_FILE, ios::app);
    if (!outf)
	{
		fprintf (stderr, "Can't open: %s\n", LOG_FILE);
		exit(1);
	}

	outf<<message<<" "<<val<<endl;

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
