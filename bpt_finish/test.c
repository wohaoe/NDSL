#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<wait.h>

int a = 2; 
int b = 3;
int c;
int main()
{
    pid_t pid;
    pid_t children[3];
     
    
    switch (pid = fork())
    {
        case 0:
        {
            for(int i=0; i < 5; i++)
            {
                printf("A");
            }
            c = a + b;
            printf("%d\n", c);
            exit(0);
        }
        case -1:
        {
            perror("can not fork child1");
            break;
        }
        default:
        {
            children[0] = pid;
            break;
        }
    } 
    switch (pid = fork())
    {
        case 0 /* constant-expression */:
        {
            for(int i=0; i < 5; i++)
            {
                printf("B");
            }
            printf("%d\n",c);
            exit(0);
        }
        case -1:
        {
            perror("can not fork child2");
            break;
        }
        default:
        {
            children[1] = pid;
            break;
        }
    } 
    switch (pid = fork())
    {
        case 0 /* constant-expression */:
        {
            for(int i=0; i < 5; i++)
            {
                printf("C");
            }
            exit(0);
        }
        case -1:
        {
            perror("can not fork child3");
            break;
        }
        default:
        {
            children[2] = pid;
            break;
        }
    } 

    if(waitpid(children[0], NULL, 0) != children[0])
	{
        perror("waitpid failed");
        exit(-1);
    }
    if(waitpid(children[1], NULL, 0) != children[1])
	{
        perror("waitpid failed");
        exit(-1);
    }
    if(waitpid(children[2], NULL, 0) != children[2])
	{
        perror("waitpid failed");
        exit(-1);
    }

    printf("\n");
    for(int j = 0; j < 3; j++)
    {
        printf("%d\n", children[j]);
    }
    printf("%d\n", c);
    return 0;
}