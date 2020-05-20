#ifndef INETADRESS_H
#define INETADRESS_H
#include <stdio.h>
#include <string>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "NetOpt.h"
struct InetAdress
{
    InetAdress() : _port(0)
    {
        memset(&_addr, 0, sizeof _addr);
    }

    bool FromIpPort(const std::string& ip, uint16_t port)
    {
        if (ip.empty())
        {
            _ip = "0.0.0.0";
        }
        _port = port;
        _addr.sin_port = htons(_port);
        _ipPortStr = _ip + ":" + std::to_string(_port);
        return NetOpt::IpStringToAddr(ip, _addr);

    }
    bool FromAddr(const sockaddr_in& addr)
    {
        _addr = addr;
        _port = ntohs(addr.sin_port);  
        bool ret = NetOpt::AddrToIpString(addr, _ip);
        _ipPortStr = _ip + ":" + std::to_string(_port);
        return ret;
    }

    sockaddr_in _addr;
    std::string _ip;
    uint16_t _port;
    std::string _ipPortStr;
};

std::ostream& operator << (std::ostream& strm, InetAdress addr)
{
    return strm << addr._ip << ":" << addr._port;
}

#endif // !INETADRESS_H
