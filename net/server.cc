#include"Socket.h"
#include"InetAddress.h"
#include"SocketsOps.h"
#include<sys/epoll.h>
#include<iostream>

using namespace dushuang;
using namespace dushuang::net;

int main(int argc, char *argv[])
{
    InetAddress address(9233);
    int listensocket = sockets::creatNonblockingOrDie();
    Socket socket(listensocket);
    socket.bindAddress(address);
    socket.listen();

    int g_epollfd;
    g_epollfd = epoll_create1(EPOLL_CLOEXEC);

    typedef std::vector<struct epoll_event> EventList;
    struct epoll_event g_event;
    EventList events(20);
    int nready;
    int buf[512] = {0};
    for(;;)
    {
        InetAddress clntAddr(9000);
        int connfd = socket.accept(clntAddr);
        g_event.data.fd = connfd;
        g_event.events = EPOLLIN;

        epoll_ctl(g_epollfd, EPOLL_CTL_ADD, connfd, &g_events);

        while(1)
        {
            nready = epoll_wait(g_epollfd, &*events.data(), static_cast<int>(events.size()), -1);
            if(nready == -1)
            {
                if(errno = EINTR)
                    continue;
                
                perror("epoll_wait failed");
                exit(1);
            }

            else if(nready == 0)
                continue;

            if((size_t) nready == events.size())
            {
                events.resize(events.size() * 2);
            }

            for()
        }

    }    
    

    
}
