#ifndef SOCKET_H
#define SOCKET_H
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
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
            SYS_ERROR_LOG << "CreateSocket error:" << strerror(errno);
        }
        return Socket(sockfd);
    }
    void ReuseAddr()
    {
        int optval = 1;
        if (0 != ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval)))
        {
            SYS_ERROR_LOG << "Reuseaddr error:" << strerror(errno);
        }

    }
    void ShutdownWrite()
    {
        if (::shutdown(_fd, SHUT_WR) < 0)
        {
            SYS_ERROR_LOG << "sockets shutdownWrite error:" << strerror(errno);
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
            SYS_ERROR_LOG << "sockets close error:" << strerror(errno);
        }
    }
    const int Fd() { return _fd; }
private:
    const int _fd;
};
#endif // !SOCKET_H