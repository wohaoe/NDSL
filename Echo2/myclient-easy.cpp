#include"std_cli.h"

int main(int argc, char **argv) {
  int listenfd;
  int n;
  int maxfdp1;

  char buf[MAXLINE], recv[MAXLINE];
  listenfd = initclient(CLIENT, argv[1]);
  printMessageln("listenfd : ", listenfd);
  str_cli(listenfd);
  printMessageln("!!!!!!!!!!!!!!!!!!!!!");
  close(listenfd);
  return 0;
}
