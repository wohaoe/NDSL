#include "my_err.h"

#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>   //read write close
#include <stdlib.h>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <cstring>    //bzero
#include <arpa/inet.h> //inet_pton
#include <malloc.h>

#define SERV_PORT 9877
#define SA struct sockaddr
#define LISTENQ 102400
#define MAXLINE 102400
#define SERVER 00
#define CLIENT 11
#define OPEN_MAX 102400
#define INFTIM -1
#define EPOLL_SIZE 102400

int init(int, char *);
size_t Read(int, void*, size_t);
void Write(int, void*, size_t);
int Accept(int, SA*, socklen_t*);
