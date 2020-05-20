#ifndef HANDLERIMP_H
#define HANDLERIMP_H
#include "Handler.h"

Handler::Handler(Epoller* poller, int fd) : _poller(poller), _fd(fd), _events(0)
{

}
void Handler::Update()
{
    _poller->Update(this);
}
int Handler:: Fd()
{
    return _fd;
}
bool Handler::IsNoEvent()
{
    return _events == 0;
}
void Handler::EnableRead()
{
    _events |= (EPOLLIN | EPOLLPRI);
    Update();
}

void Handler::EnableWrite()
{
    _events |= (EPOLLOUT);
    Update();
}


void Handler::DisableWrite()
{
    _events &= (~EPOLLOUT);
    Update();
}

void Handler::DisableAll()
{
    _events = 0;
    Update();
}
int Handler::Events()
{
    return _events;
}

void Handler::HandleEvent(const std::chrono::system_clock::time_point& timePoint, int triggerEvents)
{
    if (triggerEvents & (EPOLLIN | EPOLLHUP | EPOLLPRI))
    {
        if (_readCallback)
        {
            _readCallback(timePoint);
        }
    }
    if (triggerEvents & EPOLLOUT)
    {
        if (_writeCallback)
        {
            _writeCallback(timePoint);
        }
    }
    if (triggerEvents & EPOLLERR)
    {
        if (_errorCallback)
        {
            _errorCallback(timePoint);
        }
    }
}

void Handler::SetReadCallback(EventCallback callback)
{
    _readCallback = callback;
}
void Handler::SetWriteCallback(EventCallback callback)
{
    _writeCallback = callback;
}
void Handler::SetCloseCallback(EventCallback callback)
{
    _closeCallback = callback;
}
void Handler::SetErrorCallback(EventCallback callback)
{
    _errorCallback = callback;
}
bool Handler::IsWriting()
{
    return _events & EPOLLOUT;
}
Handler::~Handler()
{
    //DisableAll();
}
#endif // !HANDLERIMP_H

