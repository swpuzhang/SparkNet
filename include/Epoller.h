#ifndef EPOLLER_H
#define EPOLLER_H
#include <vector>
#include <map>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Log.h"
#include "date/date.h"
#include "Noncopyable.h"
using namespace std::chrono;
using namespace date;
class Handler;
class Epoller : Noncopyable
{
public:
    constexpr static int INIT_EVENT_NUM = 128;
    Epoller();
    ~Epoller();
    void Loop();
     void Update(Handler* handle);
     void Update(int operation, Handler* handle);
private:
    int _epollfd;
    std::map<int, Handler*> _handlers;
    std::vector<struct epoll_event> _events;
};
#include "HandlerImp.cpp"
#include "EpollerImp.cpp"
#endif // !EPOLLER

