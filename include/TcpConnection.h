#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include <sys/uio.h> 
#include <memory>
#include "Noncopyable.h"
#include "FunctionCall.h"
#include "Handler.h"
#include "InetAdress.h"
#include "Buffer.h"
#include "Socket.h"


class TcpConnection : Noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    enum ConnState
    {
        DIS_CONNECTED,
        CONNECTING,
        CONNECTED,
        DIS_CONNECTING,
    };
    TcpConnection(const std::string& name, Epoller* poller, int fd, const InetAdress& addr) :
        _state(CONNECTING),
        _name(name),
        _poller(poller),
        _socket(new Socket(fd)),
        _handler(new Handler(poller, fd)),
        _addr(addr)

    {
        _handler->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
        _handler->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this, std::placeholders::_1));
        _handler->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this, std::placeholders::_1));
    }
    void SetMessageCallback(const MessageCallback& callback )
    {
        _msgCallback = callback;
    }
    void SetConnectionCallback(const ConnectionCallback& callback)
    {
        _connCallback = callback;
    }
    void SetCloseCallback(const ConnectionCallback& callback)
    {
        _closeCallback = callback;
    }
    void SetWriteCompleteCallback(const WriteCompleteCallback& callback)

    {
        _writeCompeleCallback = callback;
    }
    void HandleWrite(const std::chrono::system_clock::time_point& timePoint)
    {
        if (_handler->IsWriting())
        {
            ssize_t n = write(_handler->Fd(), _outPutBuffer.ReadPos(), _outPutBuffer.ReadableBytes());
            if (n >= 0)
            {
                _outPutBuffer.Retrieve(static_cast<size_t>(n));
                if (_outPutBuffer.ReadableBytes() == 0)
                {
                    _handler->DisableWrite();
                    if (_state == DIS_CONNECTING)
                    {
                        ShutDownWrite();
                    }
                }
            }
        }
    }
    void HandleRead(const std::chrono::system_clock::time_point& timePoint)
    {
        char extrabuf[65535];
        struct iovec vec[2];
        const size_t writable = _inputBuffer.writableBytes();
        vec[0].iov_base = _inputBuffer.WritePos();
        vec[0].iov_len = writable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof(extrabuf);

        ssize_t n = readv(_handler->Fd(), vec, 2);
        if (n < 0)
        {
            if (errno != EINTR)
            {
                SYS_ERROR_LOG << "read fd " << _handler->Fd() << " ip_port " << _addr;
            }
        }
        else if (n == 0)
        {
            DEBUG_LOG << "recv n=0 from conn (" << _name << ") ipport:" << _addr;
            Close();  
        }
        else
        {
            if (writable >= static_cast<size_t>(n))
            {
                _inputBuffer.HasWrite(n);
            }
            else
            {
                _inputBuffer.HasWriteToEnd();
                _inputBuffer.AppendData(extrabuf, static_cast<size_t>(n) - writable);
            }
            _msgCallback(shared_from_this(), timePoint);
        }
    }
    void send(const void* data, size_t len)
    {
        if (_state == DIS_CONNECTED)
        {
            WARN_LOG << "already disconnected give up send";
            return;
        }
        size_t remain = len;
        size_t nwrite = 0;
        bool sendFault = false;
        if (!_handler->IsWriting() && _outPutBuffer.ReadableBytes() == 0)
        {
            nwrite = write(_handler->Fd(), data, len);
            if (nwrite < 0)
            {
                ERROR_LOG << "write size:" << nwrite << " to conn(" << _name
                    << ") " << _addr;
                if (errno != EWOULDBLOCK)
                {
                    SYS_ERROR_LOG << "send fd " << _handler->Fd() << " ip_port" << _addr;
                    if (errno == EPIPE || errno == ECONNRESET)
                    {
                        sendFault = true;
                    }
                }
            }
            else
            {
                remain = len - nwrite;
                /*if (remain == 0 && _writeCompeleCallback)
                {
                    _writeCompeleCallback(shared_from_this());
                }*/
            }
        }
        if (!sendFault && remain > 0)
        {
            _outPutBuffer.AppendData((const char*)data + nwrite, remain);
            if (!_handler->IsWriting())
            {
                _handler->EnableWrite();
            }
        }
    }
    void ShutDown()
    {
        if (_state == CONNECTED)
        {
            _state = DIS_CONNECTING;
            ShutDownWrite();
        }
    }
    void ShutDownWrite()
    {
        if (!_handler->IsWriting())
        {
            _socket->ShutdownWrite();
        }
    }
    void HandleClose(const std::chrono::system_clock::time_point& timePoint)
    {
        Close();
    }
    void Close()
    {
        _handler->DisableAll();
        if (_closeCallback)
        {
            _closeCallback(shared_from_this());
        }
    
        _state = DIS_CONNECTED;
    }
    void Start()
    {
        _state = CONNECTED;
        _handler->EnableRead();
        if (_connCallback)
        {
            _connCallback(shared_from_this());
        }
       
    }
    std::string Name() { return _name; }
    Buffer* GetBuffer() { return &_inputBuffer; }
    const InetAdress& Addr() const { return _addr; }
    const int Fd() const { return _handler->Fd(); }
private:
    ConnState _state;
    std::string _name;
    Buffer _inputBuffer;
    Buffer _outPutBuffer;
    Epoller* _poller;
    std::unique_ptr<Socket> _socket;
    std::unique_ptr<Handler> _handler;
    InetAdress _addr;
    MessageCallback _msgCallback;
    ConnectionCallback _connCallback;
    CloseCallback _closeCallback;
    WriteCompleteCallback _writeCompeleCallback;
};
#endif // !TCPCONNECTION_H
