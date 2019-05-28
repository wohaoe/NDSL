#ifndef INETADDRESS_H_
#define INETADDRESS_H_

#include<netinet/in.h>
#include<strings.h>
#include<cstring>

namespace dushuang
{
namespace net
{
class InetAddress
{
public:
    explicit InetAddress(uing16_t port = 0)
    InetAddress(string ip, uint16_t port);
    explicit InetAddress(const struct sockaddr_in& addr)
        : addr_string(addr)
    {   }

    sa_family_t family() const{return addr_.sin_family;}
    
    string toIp() const;
    string toIpPort() const;
    uint16_t toIpPort() const;

    uint32_t ipNetEndian() const;
    uint16_t portNetEndian() const{return addr_.sin_port;}

    void setSockAddr(const struct sockaddr_in& addr){addr_ = addr;}    
    const struct sockaddr* getSockAddr(){return sockets::sockaddr_cast(&addr_);}
private:
    struct sockaddr_in addr_;
}
}
}

#endif