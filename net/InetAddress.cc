#include"InetAddress.h"
#include"Endian.h"
#include"SocketsOps.h"

#include<netdb.h>
#include<netinet/in.h>
#include<stdio.h>
#include<strings.h>
#include<cstring>

using namespace dushaung;
using namespace dushuang::net;

static const in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = kInaddrAny;   //in_addr_t一般为32位的unsigned int,其字节顺序为网络字节序
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    addr_.sin_port = sockets::hostToNetwork16(port);
}

InetAddress::InetAddress(string ip, uint16_t port)
{
    bzero(&addr_, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), port, &addr_);
}

string InetAddress::toIpPort() const
{
    cahr buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

string InetAddress::toIp() const
{
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint32_t InetAddress::ipNetEndian() const
{
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toIpPort() const
{
    return sockets::networkToHost16(portNetEndian());
}

