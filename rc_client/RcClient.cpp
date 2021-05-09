#include "RcClient.h"


RcClient::RcClient()
{
    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0)
    {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

}

RcClient::~RcClient()
{
    stop();
}

void RcClient::start()
{
    // цикл по всем результатам и связывание с первым возможным
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
		return;
	}

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); //освобождаем
}

void RcClient::stop()
{
    close(sockfd);
}

//получаем sockaddr, IPv4 или IPv6:
void* RcClient::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//сокет, буфер и указатель на кол-во байт в буфере
int RcClient::sendall(int sockfd, const char *buf, int *len) //функция досылающая данные
{
    int total = 0; //сколько байт отправлено
    int bytesleft = *len; // сколько осталось
    int num;

    while(total < *len)
    {
        num = send(sockfd, buf+total, bytesleft, 0);
        if (num == -1) { break; }
        total += num;
        bytesleft -= num;
    }

    *len = total; //отправлено
    return num == -1? -1 : 0; // -1 error, 0 success
}

string RcClient::read_file(string file_name)
{
    ifstream inf;
    inf.open(file_name); //открываем файл режиме чтения
    if (!inf)
	{
		fprintf (stderr, "Can't open: %s\n", file_name);
		exit(1);
	}

	string line; //временная строка
	string content; //содержимое файла

	//Получаем вектор объектов, содержащих имя, алгоритм и хэш-сумму файлов
	while (inf)
	{
        getline(inf, line);
        content+=line;
	}

    inf.close();

    return content;
}

//читаем файл и отправляем на сервер
void RcClient::read_file_and_send(string file_name)//получаем дескриптор сокета и расположение/имя файла
{
    string data_to_send = read_file(file_name);

    int data_size = data_to_send.size();

    int bytesSend = send(sockfd, reinterpret_cast<char*>(&data_size), sizeof(int), 0);
    bytesSend = sendall(sockfd, data_to_send.c_str(), &data_size);

    if (bytesSend == -1)
    {
        perror("sendall");
        printf("Sent %d bytes because of the error!\n", data_size);
    }
}
