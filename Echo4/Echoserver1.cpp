//#include"threadpool.h"
#include "./src/ThreadPool.h"
#include "./src/CountDownLatch.h"

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

typedef std::vector<struct epoll_event> EventList;
//clock_t start = 0;

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

static void Doit(int fd);

int Accept(int);
void str_echo(int);

std::vector<int>clients;
int g_epollfd;

struct epoll_event g_event;

EventList events(20);

int countserv = 0;

int idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);

int main()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    //防止EMFILE错误
    int listenfd;

    if((listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) < 0)
    {
        ERR_EXIT("socket failed");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5160);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        ERR_EXIT("bind failed");
    }

    if(listen(listenfd, SOMAXCONN) < 0)
    {
        ERR_EXIT("listen failed");
    }

    g_epollfd = epoll_create1(EPOLL_CLOEXEC);

    g_event.data.fd = listenfd;
    g_event.events = EPOLLIN;
    epoll_ctl(g_epollfd, EPOLL_CTL_ADD, listenfd, &g_event);

    int nready;

    Reuzel::ThreadPool pool("ServerThreadPool");
    pool.setMaxQueueSize(20);
    pool.start(10);  //10个线程

    while(1)
    {
        nready = epoll_wait(g_epollfd, &*events.data(), static_cast<int>(events.size()), -1);
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

        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("nready : %d\n", nready);
        for(int i = 0; i < nready; i++)
        {
            if(events[i].data.fd == listenfd)
            {
                printf("accept\n");
                int servsock;
                servsock = Accept(listenfd);
            }
            
            if((events[i].events & EPOLLIN) && (events[i].data.fd != listenfd)) 
            {
                printf("EPOLLIN\n");
                int sock1 = events[i].data.fd;
                g_event = events[i];
                if(sock1 < 0)
                {
                    printf("sock1 < 0 \n");
                    continue;
                }
    
                epoll_ctl(g_epollfd, EPOLL_CTL_DEL, sock1, &g_event);

                pool.addTask(std::bind(Doit, sock1));
                //threadpool_add_task(&pool, Doit, (&sock1));
            }
        }
    }
   /* Reuzel::CountDownLatch latch(1);
    pool.addTask([&](){ latch.countDown(); });
    latch.wait();
    pool.stop();
    */
    return 0;
}

int Accept(int fd)
{
    int connfd;
    if((connfd = accept4(fd, NULL, NULL, O_NONBLOCK | O_CLOEXEC)) < 0)
    {
        if(errno = EMFILE)
        {
            close(idlefd);
            idlefd = accept(fd, NULL, NULL);
            close(idlefd);
            idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
        else
        {
            ERR_EXIT("accept4 failed");
        }
    }

    clients.push_back(connfd);

    g_event.data.fd = connfd;
    g_event.events = EPOLLIN;
    epoll_ctl(g_epollfd, EPOLL_CTL_ADD, connfd, &g_event);
    return connfd;
}

static void Doit(int fd)
{
//    clock_t finish = 0;
//    int time = 0.0;

    pthread_detach(pthread_self());


//    start = clock();
    
    str_echo(fd);
//    finish = clock();
//    time = finish - start;
//    printf("time : %d\n", time);
    printf("countserv : %d\n", countserv);
}

void str_echo(int fd)
{
    char buf[512] = {0};
    size_t rec = 0;
    size_t sen = 0;
    
    int epollfd = epoll_create1(O_CLOEXEC);
    
    struct epoll_event eventin;
    eventin.data.fd = fd;
    eventin.events = EPOLLIN;

    struct epoll_event eventout;
    eventout.data.fd = fd;
    eventout.events = EPOLLOUT;

    EventList eventsio(100);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &eventin);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &eventout);

    int waiteventnum = 0;

    for(;;)
    {
        waiteventnum = epoll_wait(epollfd, &*eventsio.data(), static_cast<int>(eventsio.size()), -1);
        if(waiteventnum == -1)
        {
            if(errno == EINTR)
                continue;
            
            ERR_EXIT("epoll_wait failed");
        }
        else if(waiteventnum == 0)
            continue;
        
        if((size_t)waiteventnum == eventsio.size())
        {
            eventsio.resize(eventsio.size() * 2);
        }
        
        for(int j = 0; j < waiteventnum; j++)
        {
            if(eventsio[j].events & EPOLLIN)
            {
                rec = read(fd, buf, 512);
                
                if(rec == -1)
                    ERR_EXIT("read failed");
                
                if(rec == 0)
                {
                    printf("client close\n");
                    shutdown(fd, SHUT_RD);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &eventin);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &eventout);
                    clients.erase(std::remove(clients.begin(), clients.end(), fd), clients.end());
                }
            }                

            if(eventsio[j].events & EPOLLOUT)
            {
                sen = write(fd, buf, 512);
                if(sen == -1)
                    ERR_EXIT("write failed");
                if(sen == 0)
                {
                    shutdown(fd, SHUT_WR);
                }
                countserv++;
            }
        }
    }
}
