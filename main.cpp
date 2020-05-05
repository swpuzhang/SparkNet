#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <string.h>
#include "Socket.h"
#include <cstdio>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "Json.h"
#include "Log.h"
int main()
{
    logger->init();
    Socket listenSock = Socket::CreateSocket();
    INFO_LOG << "listensock:" << listenSock.Fd();
    /*//´´½¨Ì×½Ó×Ö
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd <= 0)
    {
        std::cout << "create socket error\n";
        exit(0);
    }
    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8888);

    if (-1 == bind(listenfd, (sockaddr*)(&addr), sizeof addr))
    {
        std::cout << "bind error:" << strerror(errno) << "\n";
        exit(0);
    }
    if (-1 == listen(listenfd, 100))
    {
        std::cout << "listen error\n";
        exit(0);
    }

    int epollfd = epoll_create(100);
    if (epollfd <= 0)
    {
        std::cout << "epoll_craete errro\n";
        exit(0);
    }
    epoll_event epEve;
    epEve.events = EPOLLIN;
    epEve.data.fd = listenfd;
    if (-1 == epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &epEve))
    {
        std::cout << "add error\n";
        exit(0);
    }
    epoll_event* events = (epoll_event*)malloc(sizeof(epoll_event) * 100);
    sockaddr_in clientaddr;
    socklen_t socklen = sizeof(clientaddr);
    while (true)
    {
        int readyCount = epoll_wait(epollfd, events, 100, -1);
        if (readyCount == -1)
        {
            std::cout << "epoll_wait error:" << strerror(errno) << "\n";
            exit(0);
        }
        for (int i = 0; i < readyCount; i++)
        {

            if (listenfd == events[i].data.fd)
            {
                int clientfd = accept(listenfd, (sockaddr*)(&clientaddr), &socklen);
                if (clientfd <= 0)
                {
                    if (errno != EWOULDBLOCK && errno != EPROTO &&
                        errno != ECONNABORTED && errno != EINTR)
                    {
                        std::cout << "accept error\n";
                        exit(0);
                    }
                }
                else
                {
                    epoll_event newEve;
                    char buf[14];
                    const char* straddr = inet_ntop(AF_INET, &clientaddr.sin_addr, buf, sizeof(buf));
                    int clientport = ntohs(clientaddr.sin_port);
                    std::cout << "accept:" << straddr << ":" << clientport << std::endl;
                    newEve.events = EPOLLIN;
                    newEve.data.fd = clientfd;

                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &newEve);
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                int fd = events[i].data.fd;
                char buf[1024];
                ssize_t  nread = read(fd, buf, 1024);
                if (nread <= 0)
                {
                    if (errno != EINTR)
                    {
                        close(fd);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
                        continue;
                    }
                }
                std::cout << "recv data:" << buf << std::endl;
                write(fd, buf, nread);
            }
        }
    }*/
    return 0;
}