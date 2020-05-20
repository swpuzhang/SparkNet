#ifndef NETOPT_H
#define NETOPT_H
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
class NetOpt
{
public:
    static bool IpStringToAddr(const std::string& ip, sockaddr_in& addr)
    {
        addr.sin_family = AF_INET;
        if (ip.empty())
        {
            addr.sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
            if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
            {
                return false;
            }
        }
        
        return true;
    }
    static bool AddrToIpString(const sockaddr_in& addr, std::string& ip)
    {
        char str[INET_ADDRSTRLEN];
        if (inet_ntop(addr.sin_family, &addr.sin_addr, str, sizeof str) <= 0)
        {
            return false;
        }
        ip = str;
        return true;
    }

    static int CreateSocket()
    {
        return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    static int CreateNoBlocking()
    {
        return::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    }
    static int Connect(int fd, const sockaddr_in* addr)
    {
        return connect(fd, (sockaddr*)addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
    }

    static sockaddr_in GetPeerAddr(int sockfd)
    {
        struct sockaddr_in peeraddr;
        memset(&peeraddr, 0, sizeof peeraddr);
        socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
        if (::getpeername(sockfd, (sockaddr*)(&peeraddr), &addrlen) < 0)
        {
            SYS_ERROR_LOG << "sockets::getPeerAddr";
        }
        return peeraddr;
    }
    static int GetSocketError(int fd)
    {
        int optval;
        socklen_t optlen = static_cast<socklen_t>(sizeof optval);

        if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        {
            return errno;
        }
        else
        {
            return optval;
        }
    }
};
#endif // !NETOPT_H
