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

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

static void Doit(int);

std::vector<int>clients;
int idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);

int main()
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    int listenfd;

    if((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        ERR_EXIT("socket failed");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5189);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        ERR_EXIT("bind failed");
    }

    if(listen(listenfd, SOMAXCONN) < 0)
    {
        ERR_EXIT("listen failed");
    }

    //int nready;

    Reuzel::ThreadPool pool("ServerThreadPool");
    pool.setMaxQueueSize(20);
    pool.start(10);  //10个线程

    while(1)
    {
        pool.addTask(std::bind(Doit, listenfd));
    }

    return 0;
}

static void Doit(int _listenfd)
{
    pthread_detach(pthread_self());

    int g_epollfd;
    g_epollfd = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event g_event;
    EventList events(20);
    int connfd;
    int nready;
    int buf[512] = {0};

    if((connfd = accept4(_listenfd, NULL, NULL, O_NONBLOCK | O_CLOEXEC)) < 0)
    {
        if(errno = EMFILE)
        {
            close(idlefd);
            idlefd = accept(_listenfd, NULL, NULL);
            close(idlefd);
            idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
        else
        {
            ERR_EXIT("accept4 failed");
        }
    }

    printf("accept !!!!!!, connfd = %d\n", connfd);

    clients.push_back(connfd);

    g_event.data.fd = connfd;
    g_event.events = EPOLLIN;
    epoll_ctl(g_epollfd, EPOLL_CTL_ADD, connfd, &g_event);
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
            memset(buf, 0, 512);
            if((events[i].events & EPOLLIN)) 
            {
                printf("EPOLLIN\n");
                g_event = events[i];
                int rec = read(connfd, buf, 512);
                if(rec == -1)
                    ERR_EXIT("read failed");
                if(rec == 0)
                {
                    printf("client close\n");
                    epoll_ctl(g_epollfd, EPOLL_CTL_DEL, connfd, &g_event);
                    clients.erase(std::remove(clients.begin(), clients.end(), connfd), clients.end());
                    close(connfd);
                    return;
                }

                int sen = write(connfd, buf, 512);
                if(sen == -1)
                    ERR_EXIT("write failed");
            }
        }
    }
}