#include "RcServer.h"

RcServer::RcServer()
{
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 либо IPv6
    hints.ai_socktype = SOCK_STREAM; // потоковый сокет TCP
    hints.ai_flags = AI_PASSIVE; //мой IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) //возвр указатель на связанный список структур addrinfo
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    }

}

RcServer::~RcServer()
{

}

void RcServer::start()
{

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
            exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

//    sa.sa_handler = sigchld_handler; //обработчик мертвых процессов
//    sigemptyset(&sa.sa_mask);
//    sa.sa_flags = SA_RESTART;
//
//    if (sigaction(SIGCHLD, &sa, NULL) == -1)
//    {
//            perror("sigaction");
//            exit(EXIT_FAILURE);
//    }

    printf("server: waiting for connections...\n");

    pid_t status, childpid;
    int exit_status;

//    while(1)
//    {
        sin_size = sizeof their_addr;

        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

//    }

}

void RcServer::stop()
{
    close(new_fd);
    close(sockfd);
}

void RcServer::handleNewConn()
{

}

void RcServer::handleThisConn()
{

}

void RcServer::save_file(string file_content)
{
    string file_name = SERV_DIR;

    time_t now = time(0);
    char* dt = ctime(&now);

    file_name.append(dt);

    printf("%s\n", file_name.c_str());

    ofstream outf;
    outf.open(file_name);
    if (!outf)
	{
		fprintf (stderr, "Can't open: %s\n", file_name);
		exit(EXIT_FAILURE);
	}

	outf<<file_content;

	outf.close();
}


void RcServer::recv_file_and_save()
{
    int msg_size;

    int bytesRecv = recv(new_fd, &msg_size, sizeof(int), 0);

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

    bytesRecv = recv(new_fd, buff, msg_size, 0);

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

    save_file(string(buff, msg_size));
}

//void sigchld_handler(int s)
//{
//	(void)s;
//
//	int saved_errno = errno;
//
//	while(waitpid(-1, NULL, WNOHANG) > 0);
//
//	errno = saved_errno;
//}

//получаем sockaddr, IPv4 или IPv6:
void * RcServer::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
