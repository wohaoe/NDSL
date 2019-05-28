#include"BpTree.h"

int test = 0;
void *mem;
int size = NODE_NUMBER * 4;
int shareMomery()
{
    int shm;
    shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXO);
    if(shm == -1)
    {
        perror("can not shm_open");
        exit(-1);
    }
    ftruncate(shm, size);

    mem = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);
    if (mem == MAP_FAILED) {
        perror("can not mmap");
        exit(-1);
    }
}

int main()
{
    pid_t pid;
    pid_t children[3];
    
    /*sem_t *mutex;
    int fd1;
    fd1 = open("semdata", O_CREAT | O_RDWR | O_TRUNC, 0666);
    ftruncate(fd1, 8192);
    mutex = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    sem_init(mutex, 1, 1);
*/
    sem_t *mutex;
    mutex = sem_open("/mutex", O_CREAT, S_IRWXU | S_IRWXO, 1);    int k1 = 4;
    int k2 = 6;  
    int add_num = 0;
    string ch = "A";
    srand(time(NULL));
    int count = 0;
    int test = 0;
    BpTree bptree(KEY_NUM);
    string find_value;

   // for(int i = 0; i < 5; i++)
    //{
        if((pid = fork()) < 0)
        {
            perror("can not fork");
            exit(-1);
        }
        else if(pid == 0)
        {
            shareMomery();
            for(int j = 0; j < 100; j++)
            {
                //sem_wait(mutex);
                bptree.insert(rand() % (NODE_NUMBER * 2), ch, mutex);
                //sem_post(mutex);
            }
            
//            add_num = add(k1, k2);
            //printf("####%d####\n", add_num);

          //  shareMomery();
            //bptree.insert(rand() % (NODE_NUMBER * 2), ch, mutex);
            printf("####################after insert#################\n");
            bptree.printKeys();
            printf("############node number##############\n");
            printf("%d\n", bptree.node_number);
            for(int j = 0; j < 3; j++)
            {
                test++;
                printf("%d\n", test);
                printf("#########A%%################\n");
            }
            printf("#############child process###########\n");
            exit(0);
        }
        else
        {
            children[0] = pid;
            count++;
            usleep(1000);
        }
    //}

    
    if((pid = fork()) < 0)
    {
        perror("can not fork");
        exit(0);
    }
    else if(pid == 0)
    {
        shareMomery();
        for(int i = 0; i < 10; i++)
        {
            bptree.remove(rand() % NODE_NUMBER, mutex);
        }
        printf("#########delete success###########\n");
        exit(0);
    }
    else
    {
        children[1] = pid;
        usleep(1000);
    }
    
    if((pid = fork()) < 0)
    {
        perror("can not fork");
        exit(0);
    }
    else if(pid == 0)
    {
        shareMomery();
        for(int i = 0; i < 10; i++)
        {
            find_value = bptree.find(rand() % NODE_NUMBER, mutex);
            if(find_value != "")
            {
                printf("find value is %s\n", find_value.c_str());
            }
        }
        printf("#########find success###########\n");
        exit(0);
    }
    else
    {
        children[2] = pid;
        usleep(1000);
    }

    


   /* for(int i = 0; i < 3; i++)
    {
        printf("%d\t", children[i]);
    }
    */
    for(int i = 0; i < 3; i++)
    {
        if(waitpid(children[i], NULL, 0) != children[i])
        {
            perror("can not waitpid");
            exit(-1);
        }
    }

    munmap(mem, size);
    sem_unlink("/mutex");
    shm_unlink("/shm");
    printf("\n");
  /*  sem_destroy(mutex);
    munmap(mutex, sizeof(sem_t));
    close(fd1);*/
    exit(0);
}