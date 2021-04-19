#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include "libs.h"
#include <syslog.h>

#define MAXFD 64

int daemon_proc = 0;

// прототипы функций с проверкой на ошибки

int Socket(int domain, int type, int protocol);

void Bind(int sockfd, const sockaddr* addr, socklen_t addrlen);

void Listen(int sockfd, int backlog);

int Accept(int sockfd, sockaddr* addr, socklen_t* addrlen);

void Connect(int sockfd, const sockaddr* addr, socklen_t addrlen);

void Inet_pton(int af, const char* src, void* dst);

// обработчик сигнала
void term_handler(int i);

// инициализация демона
int daemon_init(const char *pname, int facility);

int main(int argc, char *argv[])
{
    int server = Socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in adr = { 0 };
    adr.sin_family = AF_INET;
    adr.sin_port = htons(50505);
    Bind(server, (sockaddr *) &adr, sizeof adr);
    Listen(server, 5);

    std::cout << "Server started:\n" << std::endl;

    std::ofstream fout;
    if (argc == 2)
    {
        fout.open(argv[1], std::ios_base::app);
        if (!fout.is_open())
        {
            std::cerr << "Can't open file: " << argv[1] << "\n";
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        char path[256];
        std::cout << "Enter the path to the file(\'quit\' to quit): (example: /home/user/dest.txt)\n";
        std::cin.get(path, sizeof path).get();
        if (!(strcmp(path, "q")))
            exit(EXIT_FAILURE);
        fout.open(path, std::ios_base::app);

        if (!fout.is_open())
        {
            std::cerr << "Can't open file";
            exit(EXIT_FAILURE);
        }
    }

    std::cout << "File opened successfully.\n";


    // инициализация демона
    daemon_init(argv[0], 0);

    // изменяем действия при получении
    // соответствующего сигнала
    struct sigaction sig;
    memset(&sig, 0, sizeof sig);
    sig.sa_handler = term_handler;

    sigset_t newset;
    sigemptyset(&newset);
    sigaddset(&newset, SIGHUP);
    sigaddset(&newset, SIGTERM);


    sig.sa_mask = newset;
    sigaction(SIGHUP, &sig, 0);
    sigaction(SIGTERM, &sig, 0);

    std::ostringstream os;
    char buf[256];
    int fd, ct, total;
    ssize_t nread;

    while (true)
    {
        socklen_t adrlen = sizeof adr;
        fd = Accept(server, (sockaddr*)&adr, &adrlen);
        ct = 0;
        total = 0;
        while ((nread = recv(fd, buf, sizeof buf, 0)))
        {
            ct++;
            total += sizeof buf;
            fout.write((char *) &buf, nread) << std::flush;
            memset(buf, '\0', 256);

            os << "Count: " << nread << ", part: "
                        << ct << ", total: "
                        << total << std::endl;
            strcpy(buf, os.str().c_str());
            send(fd, buf, strlen(buf), 0);
            os.str("");
            memset(buf, '\0', 256);
        }
        if (nread == -1)
        {
            perror("\nRead failed: ");
            shutdown(fd, 1);
            close(fd);
            exit(EXIT_FAILURE);
        }
        shutdown(fd, 1);
        close(fd);
    }
    close(fd);
    close(server);
    return 0;
}

int Socket(int domain, int type, int protocol)
{
    int res = socket(domain, type, protocol);
    if (res == -1)
    {
        perror("Socket failure");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Bind(int sockfd, const sockaddr* addr, socklen_t addrlen)
{
    int res = bind(sockfd, addr, addrlen);
    if (res == -1)
    {
        perror("Bind failure");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog)
{
    int res = listen(sockfd, backlog);
    if (res == -1)
    {
        perror("Listen failure");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, sockaddr* addr, socklen_t* addrlen)
{
    int res = accept(sockfd, addr, addrlen);
    if (res == -1)
    {
        perror("Accept failure");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Connect(int sockfd, const sockaddr* addr, socklen_t addrlen)
{
    int res = connect(sockfd, addr, addrlen);
    if (res == -1)
    {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, const char* src, void* dst)
{
    int res = inet_pton(af, src, dst);
    if (res == 0)
    {
        std::cerr << "inet_pton failed: src does not contain a valid string" << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (res == -1)
    {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
}

void term_handler(int i)
{
    switch (i)
    {
        case SIGHUP:
            exit(EXIT_SUCCESS);
        case SIGTERM:
            exit(EXIT_SUCCESS);
    }

}

int daemon_init(const char *pname, int facility)
{
    int i;
    pid_t pid;

    if ((pid = fork()) < 0)
        return -1;
    else if (pid)
        exit(0);

    if (setsid() < 0)
        return -1;

    signal(SIGHUP, SIG_IGN);
    if ((pid = fork()) < 0)
        return -1;
    else if (pid)
        _exit(0);

    daemon_proc = 1;

    chdir("/");

    std::cout << "\n---DAEMON IS RUNNING---\n";

    for (i = 0; i < MAXFD; i++)

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    openlog(pname, LOG_PID, facility);

    return (0);
}
