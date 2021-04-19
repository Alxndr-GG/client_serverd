#include <iostream>
#include <fstream>
#include "libs.h"

// Прототипы функций с проверкой на ошибки

int Socket(int domain, int type, int protocol);

void Bind(int sockfd, const sockaddr* addr, socklen_t addrlen);

void Listen(int sockfd, int backlog);

int Accept(int sockfd, sockaddr* addr, socklen_t* addrlen);

void Connect(int sockfd, const sockaddr* addr, socklen_t addrlen);

void Inet_pton(int af, const char* src, void* dst);

int main(int argc, char *argv[])
{
    int fd = Socket(AF_INET, SOCK_STREAM, 0); // создание сокета с протоколом TCP
    sockaddr_in adr = { 0 }; // информация об адресе и порте
    adr.sin_family = AF_INET;
    adr.sin_port = htons(50505);
    Inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr); //преобразование строки в адрес формата IPv4
    Connect(fd, (sockaddr *) &adr, sizeof adr); // установка связи с сервером

    std::ifstream fin;

    // берем аргумент из консоли как путь к файлу
    if (argc == 2)
    {
        fin.open(argv[1]);
        if (!fin)
        {
            std::cerr << "Open failed:" << argv[1] << "\n";
            exit(EXIT_FAILURE);
        }
    }

    // или указываем путь к текстовому файлу
    else
    {
        char path[256];
        std::cout << "Enter path to the file (\'q\' to quit): (example: /home/user/source.txt)\n";
        std::cin.get(path, 256).get();
        if (!(strcmp(path, "q")))
            exit(EXIT_FAILURE);
        fin.open(path);
        if (!fin)
        {
            std::cerr << "Open failed\n";
            exit(EXIT_FAILURE);
        }
    }
    std::cout << "File opened successfully.\n";
    // читаем текст из файла в буфер и отсылаем на сервер
    char buf[256];
    ssize_t nread;
    while (fin.good())
    {
        fin.read(buf, 256);
        ssize_t sz = fin.gcount();
        if (!sz)
            break;
        send(fd, buf, sz, 0);
        memset(buf, '\0', 256);

        nread = recv(fd, buf, sizeof buf, 0);
        std::cout << buf;
        memset(buf, '\0', 256);
    }

    std::cout << "Done\n";
    shutdown(fd, 1);
    close(fd);

    return 0;
}

int Socket(int domain, int type, int protocol)
{
    int res = socket(domain, type, protocol);
    if (res == -1)
    {
        perror("Socket: ");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Bind(int sockfd, const sockaddr* addr, socklen_t addrlen)
{
    int res = bind(sockfd, addr, addrlen);
    if (res == -1)
    {
        perror("Bind");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog)
{
    int res = listen(sockfd, backlog);
    if (res == -1)
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, sockaddr* addr, socklen_t* addrlen)
{
    int res = accept(sockfd, addr, addrlen);
    if (res == -1)
    {
        perror("Accept");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Connect(int sockfd, const sockaddr* addr, socklen_t addrlen)
{
    int res = connect(sockfd, addr, addrlen);
    if (res == -1)
    {
        perror("Connect");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, const char* src, void* dst)
{
    int res = inet_pton(af, src, dst);
    if (res == 0)
    {
        std::cerr << "inet_pton: src does not contain a valid string" << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (res == -1)
    {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
}
