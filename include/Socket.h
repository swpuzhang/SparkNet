#ifndef SOCKET_H
#define SOCKET_H
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "Noncopyable.h"
#include "Log.h"

class Socket
{
public:
    Socket(const int fd) : _fd(fd){}
    static Socket CreateSocket()
    {
        int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0)
        {
            SYS_CRITICAL_LOG << "CreateSocket";
        }
        return Socket(sockfd);
    }
    static Socket CreateNoBlocking()
    {
        int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sockfd < 0)
        {
            SYS_CRITICAL_LOG << "CreateSocket";
        }
        return Socket(sockfd);
    }
    void NoBlocking()
    {
        int flags = fcntl(_fd, F_GETFL, 0);
        fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
    }
    void ReuseAddr()
    {
        int optval = 1;
        if (0 != ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval)))
        {
            SYS_CRITICAL_LOG << "Reuseaddr";
        }

    }
    void ShutdownWrite()
    {
        if (::shutdown(_fd, SHUT_WR) < 0)
        {
            SYS_CRITICAL_LOG << "sockets shutdownWrite";
        }
    }

    void SetNoDelay()
    {
        int optval =  1;
        ::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY,
            &optval, static_cast<socklen_t>(sizeof optval));
    }

    void SetKeepAlive()
    {
        int optval = 1;
        ::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE,
            &optval, static_cast<socklen_t>(sizeof optval));
    }

    ~Socket()
    {
        if (::close(_fd) < 0)
        {
            SYS_CRITICAL_LOG << "sockets close error:";
        }
    }
    const int Fd() { return _fd; }
private:
    const int _fd;
};
#endif // !SOCKET_H