#include<stdio.h>
#include<iostream>
#include<fcntl.h>
#include<sys/types.h>
#include<unistd.h>

using namespace std;

int fdrd, fdwt;
char c;
    
void rdwrt()
{
    for(;;)
    {
        if(read(fdrd, &c, 1) != 1)
            return;
        write(fdwt, &c, 1); 
    }
}

int main(int argc, char **argv)
{
    fdrd = open(argv[1], O_RDONLY);
    fdwt = creat(argv[2], 0666);

    fork();
    rdwrt();
    exit(0);
    return 0;
}