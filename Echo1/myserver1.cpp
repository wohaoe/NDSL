#include"str_echo.h"
#include<sys/wait.h>

int main(int argc, char *argv[])
{
    int sock = initserver(SERVER);

    if(sock < 0)
        printMessageln("initserver failed");
        
    unsigned int childProcCount = 0;

    int sock1;
    for(;;)
    {
        sock1 = Accept(sock, NULL, NULL);

        if(sock1 > 0)
        {
            printf("accetp clntsock success\n");
            str_echo(sock1);
            break;
        }
    }

    struct addrinfo addrCriteria2;
    memset(&addrCriteria2, 0, sizeof(addrCriteria2));
    addrCriteria2.ai_family = AF_UNSPEC;
    addrCriteria2.ai_socktype = SOCK_STREAM;
    addrCriteria2.ai_protocol = IPPROTO_TCP;
    addrCriteria2.ai_flags = AI_PASSIVE;

    struct addrinfo *servAddr2;
    int rtnVal = getaddrinfo(NULL, "9932", &addrCriteria2, &servAddr2);
    if(rtnVal != 0)
    {
        err_sys("getaddrinfo failed");
    }

    int sock2 = -1;
    for(struct addrinfo *addr2 = servAddr2; addr2 != NULL; addr2 = addr2->ai_next)
    {
        sock2 = socket(addr2->ai_family, addr2->ai_socktype, addr2->ai_protocol);
        if(sock < 0)
            continue;
        
        if((bind(sock2, servAddr2->ai_addr, servAddr2->ai_addrlen) == 0)
            && (listen(sock2, 49000) == 0))
        {
            break;
        }
        close(sock2);
        sock2 = -1;
    }
    freeaddrinfo(servAddr2);

    for(;;)
    {
        int val2 = accept(sock2, NULL, NULL);
        if(val2 > 0)
        {
            printf("accept sock2 success\n");
            break;
        }
    }
    printf("sock1 is : %d\n", sock1);
    return 0;
}
