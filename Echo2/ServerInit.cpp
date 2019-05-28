#include"ServerInit.h"

int initserver(int key)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;
    addrCriteria.ai_flags = AI_PASSIVE;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, "9700", &addrCriteria, &servAddr);
    if(rtnVal != 0)
    {
        err_sys("getaddrinfo failed");
    }

    int sock = -1;
    for(struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next)
    {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(sock < 0)
            continue;
        
        if((bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) == 0)
            && (listen(sock, 49000) == 0))
        {
            break;
        }
        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}
