#include"my_err.h"

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
#include<netdb.h>

//const char* PORT = "9856";
int initclient(int ,char *);
