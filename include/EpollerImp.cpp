#ifndef EPOLLERIMP_H
#define EPOLLERIMP_H
#include "Epoller.h"
Epoller::Epoller() : _epollfd(epoll_create1(EPOLL_CLOEXEC)), _events(INIT_EVENT_NUM)
{
    if (_epollfd < 0)
    {
        SYS_CRITICAL_LOG << "epoll_create fd";
    }
}
Epoller::~Epoller()
{
    ::close(_epollfd);
}
void Epoller::
Loop()
{
    while (true)
    {
        int numEvents = ::epoll_wait(_epollfd, &*_events.begin(), (int)_events.size(), -1);
        auto tNow = steady_clock::now();
        if (numEvents < 0)
        {
            if (errno != EINTR)
            {
                SYS_ERROR_LOG << "EPOLL error";
            }
        }
        else if (numEvents > 0)
        {
            for (int i = 0; i < numEvents; i++)
            {
                Handler* handler = static_cast<Handler*>(_events[i].data.ptr);
                int events = _events[i].events;
                system_clock::time_point tNowTemp = system_clock::time_point(tNow.time_since_epoch());
                handler->HandleEvent(tNowTemp, events);
            }
            if (static_cast<int>(_events.size()) == numEvents)
            {
                _events.resize(_events.size() * 2);
            }
        }
    }
    
}
void  Epoller:: Update(Handler* handle)
{
    auto iter = _handlers.find(handle->Fd());
    if (iter != _handlers.end())
    {
        //已经存在， 采用EPOLL_CTL_MOD或者EPOLL_CTL_DEL
        if (handle->IsNoEvent())
        {
            Update(EPOLL_CTL_DEL, handle);
            _handlers.erase(iter);
        }
        else
        {
            Update(EPOLL_CTL_MOD, handle);
        }


    }
    else
    {
        //不存在， EPOLL_CTL_ADD
        if (!handle->IsNoEvent())
        {
            _handlers[handle->Fd()] = handle;
            Update(EPOLL_CTL_ADD, handle);
        }
    }
}

void Epoller::Update(int operation, Handler* handle)
{
    epoll_event event;
    memset(&event, 0, sizeof event);
    event.events = handle->Events();
    event.data.ptr = handle;
    if (epoll_ctl(_epollfd, operation, handle->Fd(), &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            SYS_ERROR_LOG << "EPOLL_CTL_DEL fd=" << handle->Fd();
        }
        else
        {
            SYS_CRITICAL_LOG << "epoll_ctl fd=" << handle->Fd();
        }
    }
}

#endif // !EPOLLERIMP_H