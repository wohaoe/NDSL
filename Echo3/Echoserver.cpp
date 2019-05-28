#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/epoll.h>
#include<time.h>

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>

#include<vector>
#include<algorithm>
#include<iostream>

typedef std::vector<struct epoll_event>EventList;

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int main()
{
    /*
    如果客户端关闭套接字close，而服务器调用了一次write，
    服务器会接收一个RST segment，如果服务器端再次调用了write，
     这个时候就会出现SIGPIPE信号*/
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    
    //防止EMFILE错误
    int idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    int listenfd;

    /*if((listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) < 0)
    {
        ERR_EXIT("socket failed");
    }*/

    if((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        ERR_EXIT("socket failed");
    }

    int flags = fcntl(listenfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = fcntl(listenfd, F_SETFL, flags);

    flags = fcntl(listenfd, F_GETFD, 0);
    flags |= O_CLOEXEC;
    ret = fcntl(listenfd, F_SETFD, flags);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5187);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        ERR_EXIT("bind failed");
    }

    if(listen(listenfd, SOMAXCONN) < 0)
    {
        ERR_EXIT("listen failed");
    }

    std::vector<int> clients;
    int epollfd;
    epollfd = epoll_create1(EPOLL_CLOEXEC);

    struct epoll_event event;
    event.data.fd = listenfd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);

	int connfd;                
    EventList events(16);

    char buf[512] = {0};
    size_t offset = 0;

 	//char *bufre = buf + offset;
               
    int nready;

    /*struct timespec timein;
    timein.tv_nsec = 0;
    timein.tv_sec = 0;

    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 0;

    struct timespec time;
    time.tv_sec = 0;
    time.tv_nsec = 0;
    */

    clock_t starttime = 0;
    clock_t finishtime = 0;
    int  time = 0.0;
    int index;

    while(1)
    {
        nready = epoll_wait(epollfd, &*events.data(), static_cast<int>(events.size()), -1);
        if(nready == -1)
        {
            if(errno == EINTR)
                continue;
            
            ERR_EXIT("epoll_wait failed");
        }
        else if(nready == 0)
            continue;
        
        if((size_t)nready == events.size())
        {
            events.resize(events.size() * 2);
        }

        for(int i = 0; i < nready; ++i)
        {
            if(events[i].data.fd == listenfd)
            {
            	
				connfd = accept4(listenfd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
                
                int optval = 0;
                socklen_t optlen = static_cast<socklen_t>(sizeof optval);
                setsockopt(connfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
                
                int optval2;
                socklen_t optlen2 = static_cast<socklen_t>(sizeof optval2);
                getsockopt(connfd, SOL_SOCKET, SO_ERROR, &optval2, &optlen2);

                printf("error:  %d\n", optval2);

                if(connfd == -1)
                {
                    if(errno = EMFILE)
                    {
                        close(idlefd);
                        idlefd = accept(listenfd, NULL, NULL);
                        close(idlefd);
                        idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
                    }
                    else
                    {
                        ERR_EXIT("accept4 failed");
                    }
                }

               /* std::cout<<"ip="<<inet_ntoa(peeraddr.sin_addr)<<
					" port="<<ntohs(peeraddr.sin_port)<<std::endl;
*/
                clients.push_back(connfd);

                event.data.fd = connfd;
                event.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
            }
            else if((events[i].events & EPOLLIN) && (events[i].data.fd == connfd))
            {
                printf("have data receive\n");
                starttime = clock();
                connfd = events[i].data.fd;
                if(connfd < 0)
                    continue;
				
				memset(buf, 0, 512);
				offset = 0;
                int ret = read(connfd, buf, 512);
                if(ret == -1)
                    ERR_EXIT("read failed");
                if(ret == 0)
                {
                    finishtime = clock();
                    std::cout<<"client close"<<std::endl;
                    close(connfd);
                    printf("close connfd !!!!!!!!!\n");
                    event = events[i];
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &event);
                    clients.erase(std::remove(clients.begin(), clients.end(), connfd), clients.end());
                    time = finishtime - starttime;
                    printf("index : %d\n", index);
                    printf("time : %d\n", time);
    
                    continue;
                }          
                  
                //std::cout<<buf;
                offset = write(connfd, buf, ret);
                if(offset < ret)
                {
                	std::cout<<"write not all"<<std::endl;
                }                
                else
                {
                    index++;
                }  
            }
        }
    }
    return 0;
}
