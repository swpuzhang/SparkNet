#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <string.h>
#include <thread>
#include <stdio.h>
#include <cstdio>
#include "Socket.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "Json.h"
#include "Log.h"
#include "Epoller.h"
#include "Handler.h"
#include "Acceptor.h"
#include "TcpServer.h"
#include "date/date.h"
#include "TcpClient.h"
using namespace date;
using namespace std::chrono;

int main()
{
    logger->init();
    Epoller poller;
    std::shared_ptr<TCPClient> client = 
        std::shared_ptr<TCPClient>(new TCPClient(&poller));
    Handler hander(&poller, fileno(stdin));
    hander.SetReadCallback([client](const system_clock::time_point& x) {
        char buf[100] = { 0 };
        char *ptr = fgets(buf, 100, stdin);
       
        if (ptr)
        {
            std::cout << "input:" << ptr << std::endl;
            client->Send(strcat(buf, "\n"), strlen(buf));
        }
        });
    hander.EnableRead();
    TcpServer server(&poller);
    server.SetConnCallback([](const TcpConnectionPtr& conn) {
        std::cout << "new conn" << conn->Addr()._ipPortStr << std::endl;
        });
    server.SetMessageCallback([](const TcpConnectionPtr& conn,
        std::chrono::system_clock::time_point tnow)
        {
            std::cout << year_month_day{ floor<days>(tnow) } << " Recv from " << conn->Addr() << " length:"
                << conn->GetBuffer()->ReadableBytes() << std::endl;
            conn->send(conn->GetBuffer()->ReadPos(), conn->GetBuffer()->ReadableBytes());
            conn->GetBuffer()->Retrieve(conn->GetBuffer()->ReadableBytes());

        });
    server.Start("", 6666);
    
    client->Connect("127.0.0.1", 6666);
    client->SetConnectionCallback([](const TcpConnectionPtr& conn)
        {
            std::cout << "connected" << std::endl;
        });
    client->SetMessageCallback([](const TcpConnectionPtr& conn,
        std::chrono::system_clock::time_point)
        {
            const char* buf = conn->GetBuffer()->ReadPos();
            size_t len = conn->GetBuffer()->ReadableBytes();
            const char* pos = std::find(buf, buf + len, '\n');
            if (pos != buf + len)
            {
               // std::cout << "recv:" << conn->GetBuffer()->ReadableBytes() << std::endl;
                std::cout << "string:" << buf;
                conn->GetBuffer()->Retrieve(pos - buf + 1);
            }
            
        });
    poller.Loop();
    
    return 0;
}