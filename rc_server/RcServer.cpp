#include "RcServer.h"

RcServer::RcServer()
{
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	}
}

RcServer::~RcServer()
{
    stop();
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

//    sa.sa_handler = sigchld_handler;
//	sigemptyset(&sa.sa_mask);
//	sa.sa_flags = SA_RESTART;

//	if (sigaction(SIGCHLD, &sa, NULL) == -1)
//	{
//		perror("sigaction");
//		exit(1);
//	}

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


        clientHandler(new_fd);

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


}

void RcServer::clientHandler(int sockfd)
{
    printf("%s\n", help_str);

    while(1)
    {
        showCurrDirName(sockfd);
        printf("$");
        int cmnd = validationInput();

        switch(cmnd)
        {
            case 1:
            {
                printf("Exiting programm\n"); sendMsgToExit(sockfd); exit(0);
            }
            case 2:
            {
                showHomeDirContent(sockfd); break;
            }
            case 3:
            {
                showCurrDirName(sockfd); break;
            }
            case 4:
            {
                changeDirTo(sockfd); break;
            }
            case 5:
            {
                showCurrDirContent(sockfd); break;
            }
            case 6:
            {
                showFileInfo(sockfd); break;
            }
            case 7:
            {
                deleteFile(sockfd); break;
            }
            case 8:
            {
                downloadFile(sockfd); break;
            }
            case 9:
            {
                uploadFile(sockfd); break;
            }
            case 0:
            {
                printf("%s\n", help_str); break;
            }
            default: break;

        }

    }

}

void RcServer::stop()
{
    close(new_fd);
    close(sockfd);
}

void RcServer::sendMsgToExit(int sockfd)
{
    sendCommand(sockfd, 1);
}

void RcServer::showHomeDirContent(int sockfd)
{
    sendCommand(sockfd, 2);
    recvFileAndShow(sockfd);
}

void RcServer::showCurrDirName(int sockfd)
{
    sendCommand(sockfd, 3);
    recvFileAndShow(sockfd);
}

void RcServer::changeDirTo(int sockfd)
{
    sendCommand(sockfd, 4);
    sendStringData(sockfd);
}

void RcServer::showCurrDirContent(int sockfd)
{
    sendCommand(sockfd, 5);
    recvFileAndShow(sockfd);
}

void RcServer::showFileInfo(int sockfd)
{
    sendCommand(sockfd, 6);
    sendStringData(sockfd);
    recvFileAndShow(sockfd);
}

void RcServer::deleteFile(int sockfd)
{
    sendCommand(sockfd, 7);
    sendStringData(sockfd);
}

void RcServer::downloadFile(int sockfd)
{
    sendCommand(sockfd, 8);
    string file_name = SERV_DIR;
    sendStringData(sockfd, file_name);
    recvFileAndSave(sockfd, file_name);
}

void RcServer::uploadFile(int sockfd)
{
    sendCommand(sockfd, 9);
    uploadFileToClient(sockfd);
}

void RcServer::sendCommand(int sockfd, int cmnd)
{
    int bytesSend = send(sockfd, reinterpret_cast<char*>(&cmnd), sizeof(int), 0);

    if(bytesSend == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }
    else if(bytesSend == 0)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }
}

void RcServer::sendStringData(int sockfd)
{
    string msg;

    getline(cin, msg);

    sendData(sockfd, msg);
}

void RcServer::sendStringData(int sockfd, string& file_name)
{
    string msg;
    getline(cin, msg);

    file_name += msg; //добавляем имя к пути чтобы знать имя файла и куда сохранить в caller'е

    sendData(sockfd, msg); //посылаем клиенту имя файла для загрузки
}


void RcServer::uploadFileToClient(int sockfd)
{
    string file_name, file_data, file_path = SERV_DIR;
    getline(cin, file_name);

    file_path+=file_name;

    sendData(sockfd, file_name);

    file_data = readFile(file_path);

    sendData(sockfd, file_data);

    cout<<recvData(sockfd)<<endl;//ответ от клиента загружено или нет
}

string RcServer::readFile(string file_name)
{
    string o_file;

    ifstream fin(file_name, ios::binary);

    if(!fin.is_open())
    {
        printf("Cannot read file");
        return "Cannot read file";
    }

    size_t file_size = fin.seekg(0, ios::end).tellg();
    fin.seekg(0);
    char * buffer = new char[file_size];

    fin.read(buffer, file_size);
    fin.close();

    o_file.assign(buffer, buffer + file_size);

    delete[] buffer;
    return o_file;
}

string RcServer::generaneRandFileName()
{
    string file_name = SERV_DIR;

    time_t now = time(0);
    char* dt = ctime(&now);

    file_name.append(dt);
    file_name.erase(file_name.size()-1, 1);

    return file_name;
}

void RcServer::recvFileAndSave(int sockfd, string file_name)
{
    string data_file = recvData(sockfd);

    ofstream ofs;
    ofs.open(file_name);

    if(!ofs.is_open())
    {
        printf("Cannot write file");
        return ;
    }

    ofs<<data_file;

    ofs.close();
}

void RcServer::recvFileAndShow(int sockfd)
{
    string info_to_show = recvData(sockfd);

    info_to_show.erase(info_to_show.size(), 1);

    cout<<info_to_show;
}

void RcServer::sendData(int sockfd, string& data)
{
    size_t data_size = data.size();

    ssize_t bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(size_t), 0);

    if(bytesSend == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }
    else if(bytesSend == 0)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }

    bytesSend = sendall(sockfd, data.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }
}


string RcServer::recvData(int sockfd)
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

    string data;
    data.assign(buff, msg_size);

    delete[] buff;

    return data;
}

void RcServer::sendErrorMsg(int sockfd, string& er_msg)
{
    er_msg+="\n";
    sendData(sockfd, er_msg);
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

void RcServer::logMsg(string message, ssize_t val)
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

int RcServer::validationInput()
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

ssize_t RcServer::sendall(int sockfd, const char *buf, size_t *len)
{
    size_t total = 0; // how much bytes was sent
    size_t bytesleft = *len; // how much bytes left
    ssize_t n;

    while(total < *len)
    {
        n = send(sockfd, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total;// really sent
    return n==-1?-1:0; // -1 error, 0 success
}
