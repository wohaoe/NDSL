#ifndef SOCKET_H_
#define SOCKE_H_

struct tcp_info

namespace dushuang
{
namespace net
{
class InetAddress;

class Socket
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {   }

    ~Socket();

    int fd() const{return sockfd_}
    bool getTcpInfo(struct tcp_info*) const;
    bool getTcpInfoString(char* buf, int len) const;

    void bindAddress(const InetAddress& localAddr);
    void listen();
    int accept(InetAddress& peerAddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReUsePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
}
}
}

#endif