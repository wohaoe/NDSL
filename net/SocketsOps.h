#ifndef SOCKETSOPS_H_
#define SOCKETSOPS_H_

#include<arpa/inet.h>

namespace dushuang
{
namespace net
{
namespace sockets
{
    void setNonBlockAndCloseOnExec(int sockfd);

    int creatNonblockingOrDie(sa_family_t family = AF_INET);
    
    void bindOrDie(int sockfd, const struct sockaddr* addr);
    int connect(int sockfd, const struct sockaddr* addr);
    void listenOrDie(int sockfd);
    int accept(int sockfd, const struct sockaddr_in* addr);

    ssize_t read(int sockfd, void* buf, size_t count);
    ssize_t write(int sockfd, void* buf, size_t count);

    ssize_t readv(int sockfd, struct iovec *iov, int iovcnt);
    
    void close(int sockfd);
    void shutdownWrite(int sockfd);

    void toIpPort(const char* buf, size_t size, const struct sockaddr* addr);
    void toIp(const char* buf, size_t size, const struct sockaddr* addr);

    void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
    
    int getSocketError(int sockfd);

    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
    const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
}
}
}
#endif