#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include "Epoller.h"
#include "Handler.h"
#include "TcpConnection.h"
#include "Socket.h"
#include "NetOpt.h"
#include <memory>

class TCPClient
{
public:
    enum ConnectState
    {
        DIS_CONNECTED,
        CONNECTING,
        CONNECTED
    };
    TCPClient(Epoller* poller) :
        _poller(poller),
        _state(DIS_CONNECTED),
        _connId(1)
    {

    }
    ~TCPClient()
    {
        /*if (_conn)
        {
            _conn->Close();
        }*/
        //由使用的客户保证内部资源的释放
    }
    bool Connect(const std::string& ip, uint16_t port)
    {
        if (_state != DIS_CONNECTED)
        {
            return true;
        }
        _addr.FromIpPort(ip, port);
        int fd = NetOpt::CreateNoBlocking();
        if (fd < 0)
        {
            SYS_ERROR_LOG << " CreateNoBlocking ";
            return false;
        }
        int ret = NetOpt::Connect(fd, &_addr._addr);
        if (ret == 0)
        {
            //立即连接成功
            ConnEstablish(fd);
        }
        else
        {
            int saveErrno = errno;
            switch (saveErrno)
            {
            case EINPROGRESS:
            case EINTR:
            case EISCONN:
                Connecting(fd);
                break;

            default:
                _state = DIS_CONNECTED;
                close(fd);
                return false;
                break;
            }
        }
        return true;
    }
    void Connecting(int fd)
    {
        _state = CONNECTING;
        _connHandler.reset(new Handler(_poller, fd));
        _connHandler->SetWriteCallback(std::bind(&TCPClient::HandleWrite,
            this,std::placeholders::_1));
        _connHandler->SetErrorCallback(std::bind(&TCPClient::HandleError,
            this, std::placeholders::_1));
        _connHandler->EnableWrite();
    }
    void HandleError(const std::chrono::system_clock::time_point& timePoint)
    {
        if (_state == CONNECTING)
        {
            int fd = _connHandler->Fd();
            //int fd = _connHandler->Fd();
            _connHandler->DisableAll();
            _connHandler.reset();
            close(fd);
            if (_disConnectCallback)
            {
                _disConnectCallback();
            }
        }
        
    }

    void HandleWrite(const std::chrono::system_clock::time_point& timePoint)
    {
        if (_state == CONNECTING)
        {
            int fd = _connHandler->Fd();
            _connHandler->DisableAll();
            _connHandler.reset();
            int err = NetOpt::GetSocketError(fd);
            if (err)
            {
                WARN_LOG << "Connect GetSocketError " << err << " " << strerror(err);
                if (_disConnectCallback)
                {
                    _disConnectCallback();
                }
            }
            else
            {
                ConnEstablish(fd);
            }
        }
    }

    void ConnEstablish(int fd)
    {
        _state = CONNECTED;
        sockaddr_in addr = NetOpt::GetPeerAddr(fd);
        InetAdress peerAddr;
        peerAddr.FromAddr(addr);
        DEBUG_LOG << "connect success peer ipport " << peerAddr;
        std::string connName = _addr._ipPortStr + "-" + std::to_string(_connId++);
        _conn = std::make_shared<TcpConnection>(connName, _poller, fd, _addr);
        _conn->SetConnectionCallback(_newConnCallback);
        _conn->SetMessageCallback(_msgCallback);
        _conn->SetCloseCallback(std::bind(&TCPClient::HandleClose, 
            this, std::placeholders::_1));
        _conn->Start();
    }
    void HandleClose(const TcpConnectionPtr& conn)
    {
        assert(conn == _conn);
        _conn.reset();
        if (_disConnectCallback)
        {
            _disConnectCallback();
        }
    }
    void Disconnect()
    {
        _state = DIS_CONNECTED;
        _conn->ShutDown();
    }
    void SetDisconnectCallback(const DisconnectCallback& callback)
    {
        _disConnectCallback = callback;
    }
    void SetConnectionCallback(const ConnectionCallback& callback)
    {
        _newConnCallback = callback;
    }
    void SetMessageCallback(const MessageCallback& msgCallback)
    {
        _msgCallback = msgCallback;
    }
    void Send(const char* buf, size_t len)
    {
        if (_conn)
        {
            _conn->send(buf, len);
        }
    }
private:
    InetAdress _addr;
    Epoller* _poller;
    std::unique_ptr<Handler> _connHandler;
    TcpConnectionPtr _conn;
    ConnectState _state;
    uint32_t _connId;
    ConnectionCallback _newConnCallback;
    DisconnectCallback _disConnectCallback;
    MessageCallback _msgCallback;
};
#endif // !TCPCLIENT_H
