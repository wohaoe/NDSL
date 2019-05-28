#include"std_cli.h"

void str_cli(int sockfd)
{
    int maxfdp1, val;
    int stdineof = 0;
    ssize_t n, nwritten;
    fd_set rset, wset;
    char to[MAXLINE], fr[MAXLINE];
    char *toiptr, *tooptr, *friptr, *froptr;

    //将三个描述符设为非阻塞
    val = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

    val = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);

    val = fcntl(STDOUT_FILENO, F_GETFL, 0);
    fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

    toiptr = tooptr = to;
    friptr = froptr = fr;

    maxfdp1 = max((max(STDIN_FILENO, STDOUT_FILENO)), sockfd) + 1;
    for(;;)
    {
        FD_ZERO(&rset); 
        FD_ZERO(&wset);

        if(stdineof == 0 && toiptr < &to[MAXLINE])
            FD_SET(STDIN_FILENO, &rset);
        if(friptr < &fr[MAXLINE])
            FD_SET(sockfd, &rset);
        if(tooptr != toiptr)
            FD_SET(sockfd, &wset);
        if(froptr != friptr)
            FD_SET(STDOUT_FILENO, &wset);
        
        select(maxfdp1, &rset, &wset, NULL, NULL);

        //从标准输入中读
        if(FD_ISSET(STDIN_FILENO, &rset))
        {
            if((n = read(STDIN_FILENO, toiptr, static_cast<size_t>(&to[MAXLINE] - toiptr))) < 0)
            {
                if(errno != EWOULDBLOCK)
                    err_sys("read error on stdin");
            }
            else if(n == 0)
            {
                //fprintf(stderr, "%s: EOF on stdin\n", gf_time());
                //如果read返回0,那么标准输入处理就此结束，我们还设置了stdineof标志，如果在to缓冲区不再有数据发送，
                //    那就调用shutdown发送FIN到服务器，如果to缓冲区仍有数据要发送，FIN的发送就得推迟到缓冲区中数据已写到套接字之后
                stdineof = 1;
                if(toiptr = tooptr)
                {
                    shutdown(sockfd, SHUT_WR);
                }
                printMessageln("read is 0!!!!!!!");
            }
            else
            {
                toiptr += n;
                printMessageln("read sdtin success, read n: ", n);
                FD_SET(sockfd, &wset);   //try and write to socket below
            }       

            printMessageln("11111111111111111111111"); 
        }
        

        //从套接字read
        if(FD_ISSET(sockfd, &rset))
        {
            if((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0)
            {
                if(errno != EWOULDBLOCK)
                    err_sys("read error on socket");
            }
            else if(n == 0)
            {
                if(stdineof)
                    return;
                else
                    err_sys("EOF not comefrom stdin");
            }
            else
            {
                friptr += n;
                printMessageln("read socket success, read n: ", n);
                FD_SET(STDOUT_FILENO, &wset);   //try and write to socket below
            }
            printMessageln("222222222222222222222");
        }
        

        //write到标准输出        
        if(FD_ISSET(STDOUT_FILENO, &wset) && (n = (friptr - froptr)) > 0)
        {
            if((nwritten = write(STDOUT_FILENO, froptr, n)) < 0)
            {
                if(errno != EWOULDBLOCK)
                    err_sys("write error to stdout");
            }
            else
            {
                froptr += nwritten;
                printMessageln("write sdtout success, write n: ", nwritten);
                if(froptr == friptr)
                    froptr = friptr = fr;
            }
            printMessageln("3333333333333333333333333");
        }

        //write到套接字
        if(FD_ISSET(sockfd, &wset) && (n = (toiptr - tooptr)) > 0)
        {
            if((nwritten = write(sockfd, tooptr, n)) < 0)
            {
                if(errno != EWOULDBLOCK)
                    err_sys("write error to socket");
            }
            else
            {
                tooptr += nwritten;
                printMessageln("write socket success, write n: ", nwritten);
                if(tooptr == toiptr)
                    tooptr = toiptr = to;
                    //如果在标准输入上遇到EOF就要发送FIN到服务器
                    if(stdineof)
                        shutdown(sockfd, SHUT_WR);
            }
            printMessageln("4444444444444444444444");
        } 
        
        /*
        if( (n = Read(fileno(stdin), to, MAXLINE)) > 0)
            Write(sockfd, to, n);
        else if(n == 0)
            break;

        if( (nwritten = Read(sockfd, fr, MAXLINE)) > 0)
            Write(fileno(stdout), fr, nwritten);
        */
    }
}