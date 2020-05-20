#ifndef HANDLER_H
#define HANDLER_H
#include <sys/epoll.h>
#include <functional>
#include <chrono>
class Epoller;

class Handler
{
public:
    using EventCallback = std::function<void(const std::chrono::system_clock::time_point& )>;
    Handler(Epoller* poller, int fd);
    ~Handler();
     void Update();
     int Fd();
     bool IsNoEvent();
     void EnableRead();
     void EnableWrite();
     void DisableWrite();
     void DisableAll();
     int Events();
     bool IsWriting();
     void HandleEvent(const std::chrono::system_clock::time_point& timePoint, int triggerEvents);
     void SetReadCallback(EventCallback callback);
     void SetWriteCallback(EventCallback callback);
     void SetCloseCallback(EventCallback callback);
     void SetErrorCallback(EventCallback callback);
private:
    Epoller* _poller;
    int _fd;
    int _events;


    EventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _closeCallback;
    EventCallback _errorCallback;
};
#include "HandlerImp.cpp"
#include "EpollerImp.cpp"
#endif // !HANDLER_H

