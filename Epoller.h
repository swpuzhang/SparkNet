#ifndef EPOLLER_H
#define EPOLLER_H
#include <vector>
#include <map>
#include <sys/epoll.h>
class Channel;
class Epoller
{
    int _epollfd;
    std::map<int, Channel*> _channels;
    std::vector<struct epoll_event> _events;
};
#endif // !EPOLLER

