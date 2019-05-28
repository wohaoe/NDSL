#include<stdio.h>
#include<iostream>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

using namespace std;

char strings[] = "hello word";

int main()
{
    int count, i;
    int to_par[2], to_chil[2];
    char buf[256];
    
    pipe(to_par);
    pipe(to_chil);

    if(fork() == 0)
    {
        close(0);
        dup(to_chil[0]);
        close(1);
        dup(to_par[1]);
        close(to_par[1]);
        close(to_chil[0]);
        close(to_par[0]);
        close(to_chil[1]);

        for(;;)
        {
            if((count = read(STDIN_FILENO, buf, sizeof(buf))) == 0)
            {
                exit(-1);
            }
            write(STDOUT_FILENO, buf, count);
        }
    }

    close(1);
    dup(to_chil[1]);
    close(0);
    dup(to_par[0]);
    close(to_chil[1]);
    close(to_par[0]);
    close(to_chil[0]);
    close(to_par[1]);

    for(int i = 0; i < 15; i++)
    {
        write(STDOUT_FILENO, strings, strlen(strings));
        read(STDIN_FILENO, buf, sizeof(buf));
    }
}