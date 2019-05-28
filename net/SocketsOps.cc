#include"SocketsOps.h"
#include"Endian.h"

#include<error.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/uio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<assert.h>
#include<iostream>

using namespace dushuang;
using namespace dushuang::net;

void setNonBlockAndCloseOnExec(int sockfd)
{
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);

    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= O_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);

int sockets::creatNonblockingOrDie(sa_family_t family = AF_INET)
{
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    
    if(sockfd < 0)
    {
        perror("creat socket failed");
        exit(1);
    }

    setNonBlockAndCloseOnExec(sockfd);
    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr* addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof *addr));

    if(ret < 0)
    {
        perror("bind failed");
        exit(1);
    }
}

void sockets::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0)
    {
        perror("listen failed");
        exit(1);
    }
}

int sockets::accept(int sockfd, const struct sockaddr_in *addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if(connfd < 0)
    {
        int savedErrno = errno;
        perror("accept failed");
        switch(savedErrno)
        {
            case EAGAIN:    //没有足够空闲的本地端口
            case ECONNABORTED:  //远程地址并没有处于监听状态
            case EINTR: //系统调用的执行由于捕获中断而中止
            case EPROTO:    //错误协议类型
            case EPERM:    //操作不允许
            case EMFILE:    //打开的文件过多
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                std::cerr<<" unexpected error of ::accept " << savedErrno;
                break;
            default:
                std::cerr<<" unknown error of ::accpet " << savedErrno;
                break;
        }
        exit(1);
    }
    return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof *addr));
}

ssize_t sockets::read(int sockfd, void* buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t sockets::write(int sockfd, void* buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, struct iovec *iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}

void sockets::close(int sockfd)
{
    if(::close(sockfd) < 0)
    {
        perror("socket close failed");
        exit(1);
    }
}

void sockets::shutdownWrite(int sockfd)
{
    if(shutdown(sockfd, SHUT_WR) < 0)
    {
        perror("socket shutdown failed");
        exit(1);
    }
}

void sockets::toIpPort(const char* buf, size_t size, const struct sockaddr* addr)
{
    toIpPort(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = sockets::networkToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port)
}

void sockets::toIp(const char* buf, size_t size, const struct sockaddr* addr)
{
    if(addr->sa_family == AF_INET)
    {
        assert(size >= INET_ADDRSTRLEN)
        {
            const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
            ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
        }
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port)
    if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        perror("fromIpPort failed");
        exit(1);
    }
}

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>(addr);
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in*>(addr);
}