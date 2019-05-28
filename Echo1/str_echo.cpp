#include "str_echo.h"

char buf[MAXLINE];
int n, wn;

void str_echo(int connfd){
  for(;;){
    if( (n = read(connfd, buf, MAXLINE)) > 0)
      // Write(connfd, buf, n);
      write(connfd, buf, n);
      break;
  }
}
