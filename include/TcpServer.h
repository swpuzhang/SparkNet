#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <memory>
#include "Epoller.h"
#include "Acceptor.h"
#include "FunctionCall.h"
#include "TcpConnection.h"

class TcpServer
{
public:
    TcpServer(Epoller* poller) : _poller(poller), _connId(1)
    {

    }
    ~TcpServer()
    {
        //由使用的客户保证内部资源的释放
    }
    void Start(const std::string& ip, uint16_t port)
    {
        _accepter = std::unique_ptr<Acceptor>(new Acceptor(_poller));
        _accepter->SetNewConnCallback(std::bind(&TcpServer::OnNewConnect, this, 
            std::placeholders::_1, std::placeholders::_2));
        if (!_accepter->Start(ip, port))
        {
            CRITICAL_LOG << "server start error";
        }
    }
    void SetConnCallback(const ConnectionCallback& callback)
    {
        _newConnCallback = callback;
    }
    void SetMessageCallback(const MessageCallback& callback)
    {
        _msgCallback = callback;
    }
private:
    void OnNewConnect(int fd, InetAdress addr)
    {
        DEBUG_LOG << "on new connect fd:" << fd << " ip_port:" << addr;
        std::string connName = addr._ipPortStr + "-" + std::to_string(_connId);
        TcpConnectionPtr connptr = std::make_shared<TcpConnection>(connName, _poller, fd, addr);
        ++_connId;
        _conns[connName] = connptr;
        connptr->SetMessageCallback(_msgCallback);
        connptr->SetCloseCallback(std::bind(&TcpServer::RemoveConn, this, std::placeholders::_1));
        connptr->SetConnectionCallback(_newConnCallback);
        connptr->Start();
    }
    void RemoveConn(const TcpConnectionPtr& conn)
    {
        DEBUG_LOG << "remove connection (" << conn->Name() << ") fd " << conn->Fd()
            << conn->Addr();
        _conns.erase(conn->Name());
    }
    ConnectionCallback _newConnCallback;
    Epoller* _poller;
    std::unique_ptr<Acceptor> _accepter;
    std::map<std::string, TcpConnectionPtr> _conns;
    MessageCallback _msgCallback;
    uint32_t _connId;
};

#endif // !TCPSERVER_H



