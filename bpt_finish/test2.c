#include"apue.h"

int glovar = 6;
char buf[] = "a write to stdout\n";

int add(int, int);

int main(void)
{
    int var;
    pid_t pid;

    var = 88;
    int num = 10;
    int add_num = 0;
    if(write(STDOUT_FILENO, buf, sizeof(buf)-1) != sizeof(buf)-1)
    {
        err_sys("write error");
    }
    printf("before fork\n");
    if((pid = fork()) < 0)
    {
        err_sys("fork error");
    }
    else if(pid == 0)
    {

        num++;
        var++;
        add_num = add(num, var);
        glovar++;
    }
    else
    {
        for(int i = 0; i < 5; i++)
        {
            num++;
        }
       // sleep(2);
    }
    printf("%d, %d, %d, %d\n", glovar, var, num, add_num);
    return 0;
}

int add(int a, int b)
{
    int c = 0;
    c = a + b;
    return c;
}