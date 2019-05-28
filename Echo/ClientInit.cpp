#include"ClientInit.h"

int initclient(int key, char *server)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(server, "9700", &addrCriteria, &servAddr);
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
        
        if(connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
            break;
        
        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}
