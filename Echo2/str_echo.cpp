#include "str_echo.h"

char buf[MAXLINE];
ssize_t n, wn;

void str_echo(int connfd){
  for(;;){
    if( (n = read(connfd, buf, MAXLINE)) > 0)
      // Write(connfd, buf, n);
      write(connfd, buf, n);
    else if(n <= 0)
      break;
  }
}
