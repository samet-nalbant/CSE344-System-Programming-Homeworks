#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
extern char *optarg;

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};
typedef union semun semun;

void *consumer(void *arg);
void *supplier(void *arg);
int semid;
void checkArguments(int argc, char **argv, char *filePath, int *c, int *n);
int c, n;
pthread_t *consumerArray;
pthread_t suplierID;
int getSemValue(int semID, int index);
void postSem(int semID, int index);
void waitSem(int semID);
int checkFile(int n, int c, char *filePath);
void handler()
{
    exit(EXIT_SUCCESS);
}
int main(int argc, char **argv)
{

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    char filePath[100];
    int error;
    void *res;

    if ((semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666)) < 0)
    {
        fprintf(stderr, "semget error");
        exit(EXIT_FAILURE);
    }

    unsigned short values[2];
    values[0] = 0;
    values[1] = 0;
    semun arg;
    arg.array = values;
    semctl(semid, 0, SETALL, arg);

    checkArguments(argc, argv, filePath, &c, &n);
    if (!checkFile(n, c, filePath))
    {
        fprintf(stderr, "Wrong N and C number!\n");
        exit(EXIT_FAILURE);
    }
    int *indexArray = (int *)malloc(sizeof(int) * c);
    consumerArray = (pthread_t *)malloc(sizeof(pthread_t) * c);
    pthread_attr_t detachedThread;
    pthread_attr_init(&detachedThread);
    pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);
    if ((error = pthread_create(&suplierID, &detachedThread, supplier, filePath)))
    {
        fprintf(stderr, "Thread couldn't created\n");
        exit(EXIT_FAILURE);
    }
    pthread_detach(suplierID);
    for (int i = 0; i < c; i++)
    {
        indexArray[i] = i;
        if ((error = pthread_create(&consumerArray[i], NULL, consumer, &indexArray[i])))
        {
            fprintf(stderr, "Thread couldn't created\n");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < c; i++)
    {
        if ((error = pthread_join(consumerArray[i], &res)))
        {
            fprintf(stderr, "Thread couldn't joined\n");
            exit(EXIT_FAILURE);
        }
    }
    pthread_attr_destroy(&detachedThread);
    free(consumerArray);
    free(indexArray);
    semctl(semid, 0, IPC_RMID);
    return 0;
}
void checkArguments(int argc, char **argv, char *filePath, int *c, int *n)
{
    int option;
    if (argc != 7)
    {
        fprintf(stderr, "Invalid Argument Number!\n");
        exit(EXIT_FAILURE);
    }
    while ((option = getopt(argc, argv, "C:N:F:")) != -1)
    {
        switch (option)
        {
        case 'C':
            *c = atoi(optarg);
            break;
        case 'N':
            *n = atoi(optarg);
            break;
        case 'F':
            strcpy(filePath, optarg);
            break;
        default:
            fprintf(stderr, "Invalid Arguments\n");
            exit(EXIT_FAILURE);
        }
    }
    if (filePath == NULL || *n <= 1 || *c <= 4)
    {
        fprintf(stderr, "Invalid Arguments\n");
        exit(EXIT_FAILURE);
    }
}
void *consumer(void *arg)
{
    time_t t;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    int oneCount, twoCount;
    char temp[50];

    for (int i = 0; i < n; i++)
    {
        oneCount = getSemValue(semid, 0);
        twoCount = getSemValue(semid, 1);
        time(&t);
        sprintf(temp, "%s", ctime(&t));
        temp[strlen(temp) - 1] = '\0';
        printf("%s ----- Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2’.\n", temp, *(int *)arg, i, oneCount, twoCount);
        fflush(stdout);
        waitSem(semid);
        oneCount = getSemValue(semid, 0);
        twoCount = getSemValue(semid, 1);
        time(&t);
        sprintf(temp, "%s", ctime(&t));
        temp[strlen(temp) - 1] = '\0';
        printf("%s ----- Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x ‘1’,%d x ‘2’.\n", temp, *(int *)arg, i, oneCount, twoCount);
        fflush(stdout);
    }
    time(&t);
    sprintf(temp, "%s", ctime(&t));
    temp[strlen(temp) - 1] = '\0';
    printf("%s ----- Consumer-%d has left.\n", temp, *(int *)arg);
    fflush(stdout);
    return NULL;
}

void *supplier(void *arg)
{
    time_t t;
    char ch;
    char temp[50];
    int oneCount, twoCount;
    int fd = open((char *)arg, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        fprintf(stderr, "Open File\n");
        exit(EXIT_FAILURE);
    }
    int sz = 1;
    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        if (sz > 0)
        {

            if (ch == '1')
            {
                oneCount = getSemValue(semid, 0);
                twoCount = getSemValue(semid, 1);
                time(&t);
                sprintf(temp, "%s", ctime(&t));
                temp[strlen(temp) - 1] = '\0';
                printf("%s ----- Supplier: read from input a ‘%c’. Current amounts: %d x ‘1’, %d x ‘2’\n", temp, ch, oneCount, twoCount);
                fflush(stdout);
                postSem(semid, 0);
                oneCount = getSemValue(semid, 0);
                twoCount = getSemValue(semid, 1);
                time(&t);
                sprintf(temp, "%s", ctime(&t));
                temp[strlen(temp) - 1] = '\0';
                printf("%s ----- Supplier: delivered a ‘1’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n", temp, oneCount, twoCount);
                fflush(stdout);
            }
            else if (ch == '2')
            {
                oneCount = getSemValue(semid, 0);
                twoCount = getSemValue(semid, 1);
                time(&t);
                sprintf(temp, "%s", ctime(&t));
                temp[strlen(temp) - 1] = '\0';
                printf("%s ----- Supplier: read from input a ‘%c’. Current amounts: %d x ‘1’, %d x ‘2’\n", temp, ch, oneCount, twoCount);
                fflush(stdout);
                postSem(semid, 1);
                oneCount = getSemValue(semid, 0);
                twoCount = getSemValue(semid, 1);
                time(&t);
                sprintf(temp, "%s", ctime(&t));
                temp[strlen(temp) - 1] = '\0';
                printf("%s ----- Supplier: delivered a ‘2’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n", temp, oneCount, twoCount);
                fflush(stdout);
            }
            else
            {
                continue;
            }
        }
    }
    time(&t);
    sprintf(temp, "%s", ctime(&t));
    temp[strlen(temp) - 1] = '\0';
    printf("%s ----- The Supplier has left.\n", temp);
    fflush(stdout);
    return NULL;
}
int getSemValue(int semID, int index)
{
    semun arg;
    return semctl(semID, index, GETVAL, arg);
}
void postSem(int semID, int index)
{
    struct sembuf sb;
    sb.sem_num = index; // interested semaphore index
    sb.sem_op = 1;      // increasing value
    sb.sem_flg = 0;
    semop(semID, &sb, 1); // sem post
}
void waitSem(int semID)
{
    struct sembuf sb[2];
    sb[0].sem_num = 0;
    sb[1].sem_num = 1;
    sb[0].sem_op = -1;
    sb[1].sem_op = -1;
    sb[0].sem_flg = 0;
    sb[1].sem_flg = 0;
    semop(semid, sb, 2);
}
int checkFile(int n, int c, char *filePath)
{
    int sz = 1;
    int count = 0;
    char ch;
    int fd = open(filePath, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        fprintf(stderr, "Open File");
        exit(EXIT_FAILURE);
    }
    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        if (sz > 0)
        {
            if (ch == '1')
            {
                count++;
            }
        }
    }
    close(fd);
    if (count != n * c)
    {
        return 0;
    }
    return 1;
}