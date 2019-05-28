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

int main(void)
{
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");
	std::cout<<"sock = "<<sock<<std::endl;
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5189);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.9");

	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{	
		ERR_EXIT("connect");
	}

	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(localaddr);
	if (getsockname(sock, (struct sockaddr*)&localaddr, &addrlen) < 0)
	ERR_EXIT("getsockname");

	std::cout<<"ip="<<inet_ntoa(localaddr.sin_addr)<<
		" port="<<ntohs(localaddr.sin_port)<<std::endl;
        

	char sendbuf[512] = {0};
	char recvbuf[512] = {0};
	
	int fd1 = open("in.txt", O_RDONLY);
	int fd2 = open("out.txt", O_RDWR);

	long int offset1 = 0;
	long int offset2 = 0;
	int num1 = 0;
	int num2 = 0;
	int num3 = 0;
	long int index = 0;

	while ((num1 = read(fd1, sendbuf, 512)) > 0)
	{
		offset1 += num1;
		write(sock, sendbuf, num1);
		if(num2 = read(sock, recvbuf, sizeof(recvbuf)) > 0)
		{
			/*for(int i = 0; i < num2; i++)
			{
				printf("%c\t", recvbuf[i]);
			}
			*/
			index++;
			//break;
		}

		//fputs(recvbuf, stdout);
		num3 = pwrite(fd2, recvbuf, num2, offset2);
		offset2 += num3;

		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}
	printf("index = %ld\n", index);
	close(sock);
	
	return 0;
}
