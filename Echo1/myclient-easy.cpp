#include "my_unp.h"
#include"ClientInit.h"


int main(int argc, char **argv) {
  int listenfd;
  int n;
  char buf[MAXLINE], recv[MAXLINE];
  listenfd = initclient(CLIENT, argv[1]);

  for(;;){
    if( (n = Read(fileno(stdin), buf, MAXLINE)) > 0)
        Write(listenfd, buf, n);
    else if(n == 0)
      break;

    if( (n = Read(listenfd, recv, MAXLINE)) > 0)
      Write(fileno(stdout), recv, n);

    if(getc(stdin) == 'q')
    {
      break;
    }
  }

  int sock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  short port2 = 9932;
  struct sockaddr_in addr2;
  bzero(&addr2, sizeof(addr2));
  addr2.sin_family = AF_INET;
  addr2.sin_port = htons(port2);
  int rtnval = inet_pton(AF_INET, argv[1], &addr2.sin_addr.s_addr);

  if(connect(sock2, (struct sockaddr *)&addr2, (socklen_t)sizeof(addr2)) == 0);
  {
    printf("listenfd2 connect success\n");
  }

  return 0;
}
