#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include <functional>
#include "Socket.h"
#include "Handler.h"
#include "InetAdress.h"
#include "Epoller.h"

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAdress&)> ;
    Acceptor(Epoller* poller) : 
        _poller(poller),
        _socket(Socket::CreateNoBlocking()), 
        _handler(_poller, _socket.Fd()),
        _idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
    {
        if (_socket.Fd() < 0 || _idlefd < 0)
        {
            SYS_CRITICAL_LOG << "create acceptro socket";
        }
        _socket.ReuseAddr();
        _socket.SetNoDelay();
    }
    ~Acceptor()
    {
        _handler.DisableAll();
        ::close(_idlefd);
        ::close(_socket.Fd());
    }
    bool  Start(const std::string& ip, const uint16_t port)
    {
        if (!_addr.FromIpPort(ip, port))
        {
            return false;
        }
        if(::bind(_socket.Fd(), (sockaddr*)&_addr._addr, sizeof(_addr._addr)) != 0)
        {
            SYS_ERROR_LOG << "bind ip " << _addr._ip << ":" << _addr._port;
            return false;
        }
        if (::listen(_socket.Fd(), SOMAXCONN) != 0)
        {
            SYS_ERROR_LOG << "listen";
            return false;
        }
        _handler.SetReadCallback(std::bind(&Acceptor::HandleRead, this, std::placeholders::_1));
        _handler.EnableRead();
        return true;
    }
    void SetNewConnCallback(const NewConnectionCallback& callback) { _newConnCallback = callback; }

private:
    void HandleRead(system_clock::time_point ttime)
    {
        InetAdress peerAddr;
        socklen_t len = sizeof(peerAddr._addr);
        int connfd = ::accept(_socket.Fd(), (sockaddr*)&peerAddr._addr, &len);
        if (connfd >= 0)
        {
            peerAddr.FromAddr(peerAddr._addr);
            if (_newConnCallback)
            {
                _newConnCallback(connfd, peerAddr);
            }
            else
            {
                ::close(_socket.Fd());
            }
        }
        else
        {
            //当文件描述符不够时， 优雅关闭套接字
            SYS_ERROR_LOG << "accept";
            if (errno == EMFILE)
            {
                ::close(_idlefd);
                _idlefd = ::accept(_socket.Fd(), NULL, NULL);
                ::close(_idlefd);
                _idlefd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
            }
            //
        }

    }
   
private:

    Epoller* _poller;
    Socket _socket;
    Handler _handler;
    InetAdress _addr;
    NewConnectionCallback _newConnCallback;
    int _idlefd;
};

#endif // !ACCEPTOR_H

