#include "RcClient.h"


RcClient::RcClient()
{
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0)
    {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	}
}

RcClient::~RcClient()
{
    stop();
}

void RcClient::start()
{
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
	}

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);

    server_handler(sockfd);
}

void RcClient::stop()
{
    close(sockfd);
}

void RcClient::show_home_dir_content(int sockfd)
{
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
        if(temp[0] == '.') continue;//hide .name files

        data_to_send.append(entry->d_name).append("\n");
    }

    sendData(sockfd, data_to_send);
}

void RcClient::show_cur_dir_name(int sockfd)
{
    int buf_size = 1024;
    char *buf;

    buf = getcwd (NULL, buf_size);

    if (buf == NULL)
    {
        fprintf (stderr, "getcwd() error\n");
        return;
    }

    sendData(sockfd, buf);

    free (buf);
}

void RcClient::change_dir_to(int sockfd)
{
    string data = recvData(sockfd);

    if (chdir(data.c_str()) == -1)
    {
        fprintf (stderr, "chdir() error\n");
        return;
    }
}

void RcClient::show_cur_dir_content(int sockfd)
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

    sendData(sockfd, data_to_send);

    closedir (dir);
    free (buf);

}

void RcClient::show_file_detail_info(int sockfd)
{
    struct stat st;

    string data = recvData(sockfd);

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

    sendData(sockfd, data_to_send);

}

void RcClient::delete_file(int sockfd)
{
    string data = recvData(sockfd);

    if (unlink (data.c_str()) == -1)
    {
        fprintf (stderr, "Cannot unlink file (%s)\n", data.c_str());
        return;
    }

}

void RcClient::upload_file(int sockfd)
{
    string file_to_upload = recvData(sockfd);

    read_file_and_send(sockfd, file_to_upload);
}

void RcClient::download_file(int sockfd)
{
    downloadFileFromServer(sockfd);
}

void RcClient::server_handler(int sockfd)
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
                show_home_dir_content(sockfd); break;
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
                upload_file(sockfd); break;
            }
            case 9:
            {
                download_file(sockfd); break;
            }
            default: break;

        }

    }

}

string RcClient::read_file(string file_name)
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
    char * buff = new char[file_size];

    fin.read(buff, file_size);
    fin.close();

    o_file.assign(buff, buff + file_size);
    delete[] buff;

    return o_file;
}


void RcClient::downloadFileFromServer(int sockfd)
{
    string file_name = recvData(sockfd);

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

string RcClient::generate_file_name()
{
    string file_name;

    time_t now = time(0);
    char* dt = ctime(&now);

    file_name.append(dt);
    file_name.erase(file_name.size()-1, 1);

    return file_name;

}

void RcClient::read_file_and_send(int sockfd, string file_to_upload)
{
    string data_to_send = read_file(file_to_upload);

    sendData(sockfd, data_to_send);
}


void RcClient::sendData(int sockfd, string data)
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


string RcClient::recvData(int sockfd)
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

//получаем sockaddr, IPv4 или IPv6:
void* RcClient::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

ssize_t RcClient::sendall(int sockfd, const char *buf, size_t *len)
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

