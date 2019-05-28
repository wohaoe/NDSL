#include "./src/ThreadPool.h"
#include "./src/CountDownLatch.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include<fcntl.h>

#include <iostream>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)
static void callback(int);
void Connect(int);
void str_cli(int);

//long int offset2 = 0;

int main()
{
   // int sock;
    //if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    //ERR_EXIT("socket");
    int *sock;
    sock = (int *)malloc(sizeof(int));
    for(int i = 0; i < 10; i++)
    {
        if ((sock[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            ERR_EXIT("socket");
    }

    Reuzel::ThreadPool pool("ClientThreadPool");
    pool.setMaxQueueSize(20);
    pool.start(10);  //10个线程

    int fd1 = open("in.txt", O_RDONLY);
	int fd2 = open("out.txt", O_RDWR);

    for(int i=0; i < 10; i++)
    {
        Connect(sock[i]);
        pool.addTask(std::bind(str_cli, sock[i]));
    }
    
    sleep(20);
   
    for(int i = 0; i < 10; i++)
    {
        close(sock[i]);
    }

    free(sock);
    return 0;
}

void Connect(int fd)
{
    struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5183);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.3");

    if (connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    ERR_EXIT("connect");
}

void str_cli(int fd)
{
    char sendbuf[512] = {0};
    char recvbuf[512] = {0};

    long int offset1 = 0;
    int num1 = 0;
    int num2 = 0;
    //int num3 = 0;

    long int count;

	int fd1 = open("in.txt", O_RDONLY);
//	int fd2 = open("out.txt", O_RDWR);

    while ((num1 = pread(fd1, sendbuf, 512, offset1)) > 0)
	{
		offset1 += num1;
		write(fd, sendbuf, num1);
        count++;
		num2 = read(fd, recvbuf, sizeof(recvbuf));

//		num3 = pwrite(fd2, recvbuf, num2, offset2);
//		offset2 += num3;

		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}
    printf("count : %ld\n", count);

}

static void callback(int sockclient)
{
    pthread_detach(pthread_self());

    str_cli(sockclient);
}