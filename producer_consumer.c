#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include<sys/wait.h>

#define N 10

sem_t *mutex;

/* who read head already
 * 1 if has not read yet
 * 0 if has already read */
sem_t *readA, *readB, *readC;

/* producers and consumers semaphores */
sem_t *prodA, *prodB, *consA, *consB, *consC;

//int head = 0;
//int tail = 0;

void* mem;
//void* elements;
int num;
char buf[100];
int fd;

void producerA();
void producerB();
void consumerA();
void consumerB();
void consumerC();
void updateSize(int diff);
void printBuffer();

//char generateRandomLetter();

typedef struct Variables {
    int size;
    int head;
    int tail;
	int flag;
    int flag1;
} Variables;

Variables* vars;

typedef struct Data {
    int A, B, C; // read by A, B, C

    char val;
} Data;

size_t size = N*sizeof(Data);

int sharedMemory()
{
    int shm;
    int eshm;
    void* tmp;
    shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXO);    //共享内存

    if (shm == -1) {
        printf("Could not create shared memory, errno: %d\n", errno);
        return 1;
    }

    ftruncate(shm, size);   //将文件大小改变为参数length指定的大小
    mem = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);

    if (mem == MAP_FAILED) {
        printf("Could not map shared memory, errno: %d\n", errno);
        return 1;
    }

    eshm = shm_open("/eshm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXO);

    if (eshm == -1){
        printf("Could not create shared memory (eshm), errno: %d\n", errno);
        return 1;
    }

    ftruncate(eshm, sizeof(Variables));
    tmp = mmap(NULL, sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, eshm, 0);
    if (tmp == MAP_FAILED) {
        printf("Could not map shared memory (vars)), errno: %d \n", errno);
        return 1;
    }
    vars = (Variables*)tmp;
}

char generateRandomLetter(){
    char ch = buf[vars->flag];
	vars->flag++;
	return ch;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    pid_t children[5];
    time_t t;
    char ch;
	
	if(argc != 2)
    {
        printf("Usage:<pathname>\n");
        exit(0);
    }
    
    fd = open(argv[1], O_RDONLY);
    if(fd < 0)
    {
        perror("open failed");
		exit(-1);
    }
    else
    {
        if((num = read(fd, buf, size)) < 0)
        {
            perror("read failed");
			exit(-1);
        }
    }

    mutex = sem_open("/mutex", O_CREAT, S_IRWXU | S_IRWXO, 1);  //posix信号量

    readA = sem_open("/readA", O_CREAT, S_IRWXU | S_IRWXO, 1);
    readB = sem_open("/readB", O_CREAT, S_IRWXU | S_IRWXO, 1);
    readC = sem_open("/readC", O_CREAT, S_IRWXU | S_IRWXO, 1);

    prodA = sem_open("/prodA", O_CREAT, S_IRWXU | S_IRWXO, 0);
    prodB = sem_open("/prodB", O_CREAT, S_IRWXU | S_IRWXO, 0);

    consA = sem_open("/consA", O_CREAT, S_IRWXU | S_IRWXO, 0);
    consB = sem_open("/consB", O_CREAT, S_IRWXU | S_IRWXO, 0);
    consC = sem_open("/consC", O_CREAT, S_IRWXU | S_IRWXO, 0);

    srand((unsigned) time(&t));

    printf("Forks starting now...\n");

    switch(pid = fork())
    {
        case 0:
            printf("ProducerA...\n");
            sharedMemory();
            producerA();
            exit(0);
        case -1:
            printf("Could not fork, errno %d\n", errno);
            break;
        default:
            children[0] = pid;
            break;
    }

    switch(pid = fork())
    {
        case 0:
            printf("ProducerB...\n");
            sharedMemory();
            producerB();
            exit(0);
        case -1:
            printf("Could not fork, errno %d\n", errno);
            break;
        default:
            children[1] = pid;
            break;
    }

    switch(pid = fork())
    {
        case 0:
            printf("ConsumerA...\n");
            sharedMemory();
            consumerA();
            exit(0);
        case -1:
            printf("Could not fork, errno %d\n", errno);
            break;
        default:
            children[2] = pid;
            break;
    }

    switch(pid = fork())
    {
        case 0:
            printf("ConsumerB... \n");
            sharedMemory();
            consumerB();
            exit(0);
        case -1:
            printf("Could not fork, errno %d\n", errno);
            break;
        default:
            children[3] = pid;
            break;
    }

    switch(pid = fork())
    {
        case 0:
            printf("ConsumerC... \n");
            sharedMemory();
            consumerC();
            exit(0);
        case -1:
            printf("Could not fork, errno %d\n", errno);
            break;
        default:
            children[4] = pid;
            break;
    }


    /* CLEANUP SECTION */
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
    if(waitpid(children[3], NULL, 0) != children[3])
	{
        perror("waitpid failed");
        exit(-1);
    }
    if(waitpid(children[4], NULL, 0) != children[4])
	{
        perror("waitpid failed");
        exit(-1);
    }

    kill(children[0], SIGKILL);
    kill(children[1], SIGKILL);
    kill(children[2], SIGKILL);
    kill(children[3], SIGKILL);
    kill(children[4], SIGKILL);

    munmap(mem, size);
    munmap(vars, sizeof(Variables));

    sem_unlink("/mutex");
    sem_unlink("/readA");
    sem_unlink("/readB");
    sem_unlink("/readC");
    sem_unlink("/consA");
    sem_unlink("/consB");
    sem_unlink("/consC");
    sem_unlink("/prodA");
    sem_unlink("/prodB");

    shm_unlink("/shm");
    shm_unlink("/eshm");

}

void updateSize(int diff){
    int prev_size;
    int new_size;

    prev_size = vars->size;

    vars->size += diff;

    new_size = vars->size;

    if (prev_size == N && new_size < N){
        sem_post(prodA);
    }

    if (new_size <= N-2 && prev_size > N-2) {
        sem_post(prodB);
    }

    // mozna byloby wprowadzic losowanie ktory jako pierwszy zostanie postawiony na nogi
    if (prev_size == 0 && new_size > 0){
        sem_post(consA);
        sem_post(consB);
        sem_post(consC);
    }
}

void producerA() {
    while(1){
        usleep(rand() % 300);
		if(vars->flag == num)
        {
            break;
        }
        printf("Mutex prodA\n");
        sem_wait(mutex);

        int size = vars->size; // get current size of buffer

        if (size < N){
            Data * newData = (Data*) malloc(sizeof(Data));
            newData->val = generateRandomLetter();
            newData->A = 0;
            newData->B = 0;
            newData->C = 0;

            printf("producerA: pushing letter: %c \n", newData->val);

            *((Data*) mem + vars->tail) = *newData;

            vars->tail = (vars->tail + 1) % N;

            updateSize(1);
            sem_post(mutex);
        }
        else {
            printf("PROD A: \n");
            printBuffer();
            sem_post(mutex);
            sem_wait(prodA);
            continue;
        }
    }
}

void producerB() {
    while(1){
        usleep(rand() % 200);
        if(vars->flag == num)
        {
            break;
        }
		printf("Mutex prodB\n");
        sem_wait(mutex);

        int size = vars->size;

        if (size <= N-2) {
            Data * newData = (Data*) malloc(sizeof(Data));
            Data * newData2 =(Data*) malloc(sizeof(Data));

            newData->val = generateRandomLetter();
            newData->A = 0;
            newData->B = 0;
            newData->C = 0;

            newData2->val = generateRandomLetter();
            newData2->A = 0;
            newData2->B = 0;
            newData2->C = 0;

            printf("producerB: pushing letters: %c and %c\n", newData->val, newData2->val);

            *((Data*) mem + vars->tail) = *newData;
            vars->tail = (vars->tail +1) % N;
            *((Data*) mem + vars->tail) = *newData2;
            vars->tail = (vars->tail +1) % N;

            updateSize(2);
            sem_post(mutex);
        }
        else {
            printf("PROD B: \n");
            printBuffer();
            sem_post(mutex);
            sem_wait(prodB);
            printf("Not waiting\n");
            continue;
        }
    }
}

void consumerA() {
    while(1) {
        usleep(rand() % 100);
        if(vars->flag1 == num)
        {
            break;
        }
		sem_wait(readA);
        printf("Mutex consA\n");
        sem_wait(mutex);

        int size = vars->size;

        if (size > 0) {

            Data *readElement = ((Data *) mem + vars->head);

            if (!readElement->A && readElement->B) {
                printBuffer();
                printf("consumerA: delete letter: %c \n", readElement->val);

                ((Data*) mem + vars->head)->val = '&';

                vars->head = (vars->head + 1) % N;
                vars->flag1++;

                if (readElement->C) sem_post(readC);

                sem_post(readB);
                sem_post(readA);
                updateSize(-1);
                sem_post(mutex);
            } else {
                printBuffer();
                printf("consumerA: read letter: %c \n", readElement->val);
                readElement->A = 1;

                sem_post(mutex);
            }

        }
        else {
            printf("CONS A: \n");
            printBuffer();
            sem_post(readA);
            sem_post(mutex);
            sem_wait(consA);
            continue;
        }
    }
}

void consumerB() {
    while(1) {
        usleep(rand() % 100);
        if(vars->flag1 == num)
        {
            break;
        }
		sem_wait(readB);
        printf("Mutex consB\n");
        sem_wait(mutex);

        int size = vars->size;

        if (size > 0) {

            Data *readElement = ((Data *) mem + vars->head);

            if (!readElement->B && readElement->A) {
                printBuffer();
                printf("consumerB: delete letter: %c \n", readElement->val);

                ((Data*) mem + vars->head)->val = '&';

                vars->head = (vars->head + 1) % N;
                vars->flag1++;
                
                if (readElement->C) sem_post(readC);

                sem_post(readA);
                sem_post(readB);
                updateSize(-1);
                sem_post(mutex);
            } else {
                printBuffer();
                printf("consumerB: read letter: %c \n", readElement->val);
                readElement->B = 1;

                sem_post(mutex);
            }

        }
        else {
            printf("CONS B: \n");
            printBuffer();
            sem_post(readB);
            sem_post(mutex);
            sem_wait(consB);
            continue;
        }
    }
}

void consumerC(){
    while(1) {
        usleep(rand() % 200);
     	if(vars->flag1 == num)
        {
            break;
        }
        sem_wait(readC);
        printf("Mutex consC\n");
        sem_wait(mutex);

        int size = vars->size;

        if (size > 0) {

            Data *readElement = ((Data *) mem + vars->head);

            if (!readElement->B && !readElement->A) {
                printBuffer();
                printf("consumerC: delete letter: %c \n", readElement->val);

                ((Data*) mem + vars->head)->val = '&';

                vars->head = (vars->head + 1) % N;
                vars->flag1++;

                sem_post(readC);
                updateSize(-1);
                sem_post(mutex);
            } else {
                printBuffer();
                printf("consumerC: read letter: %c \n", readElement->val);
                readElement->C = 1;

                sem_post(mutex);
            }

        }
        else {
            printf("CONS C: \n");
            printBuffer();
            sem_post(readC);
            sem_post(mutex);
            sem_wait(consC);
            continue;
        }
    }
}

void printBuffer(){
    int size, i;
    size = vars->size;
    i = 0;

    printf("SIZE: %d HEAD: %d TAIL: %d FLAG: %d FLAG1:%d\n", size, vars->head, vars->tail, vars->flag, vars->flag1);

    while (i < size){
        printf("%c ", ((Data *) mem + (i + vars->head) % N)->val);
        ++i;
    }

    while (size < N){
        printf("-- ");
        ++size;
    }

    printf("\n");
}
