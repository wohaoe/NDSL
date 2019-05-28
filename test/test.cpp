#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<iostream>
#include<strings.h>

using namespace std;

const int MAXSIZE = 8;

int main()
{
    int fd;
    if((fd = open("test.txt", O_RDONLY)) < 0)
    {
        cout << "open error" << endl;
        return 0;
    }

    ssize_t readnum1;
    ssize_t readnum2;
    char buf1[MAXSIZE];
    char buf2[MAXSIZE];

    bzero(buf1, sizeof(buf1));
    bzero(buf2, sizeof(buf2));

    if((readnum1 = pread(fd, buf1, MAXSIZE, 0)) < 0)
    {
        cout << "read error" << endl;
        return 0;
    }
    else
    {
        for(int i = 0; i < readnum1; i++)
        {
            cout << buf1[i];
        }
        cout << endl;
    }
    
    sleep(2);

    if((readnum2 = pread(fd, buf2, MAXSIZE, 0)) < 0)
    {
        cout << "read error" << endl;
        return 0;
    }
    else
    {
        for(int i = 0; i < readnum2; i++)
        {
            cout << buf2[i];
        }
        cout << endl;
    }

    return 0;
}