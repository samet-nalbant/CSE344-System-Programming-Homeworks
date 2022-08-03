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
#include "fifo_seqnum.h"
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>
#include "become_daemon.h"

#define CLIENT_FIFO "/tmp/client%d"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO) + 20)

sem_t *handledSemaphore;
sem_t *invertedSemaphore;
sem_t *busy;
sem_t *last;
sem_t *sigintSemaphore;
int fd;
extern char *optarg;
request *create_shared_memory(size_t size)
{
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    return (request *)mmap(NULL, size, protection, visibility, -1, 0);
}

int isInvertible(int **matrix, int n);
int determinantOfMatrix(int **matrix, int n);
void getCofactor(int **matrix, int **temp, int p, int q, int n);
int getMatrixSize(char *charMatrix);

char clientFifo[CLIENT_FIFO_NAME_LEN];
void handler(int sig_num)
{
    sem_post(sigintSemaphore);
    int count;
    sem_getvalue(sigintSemaphore, &count);
    shm_unlink("myshm");
    if (count == 1)
    {
        int handled, inverted;
        char temp[100];
        sem_getvalue(handledSemaphore, &handled);
        sem_getvalue(invertedSemaphore, &inverted);
        sprintf(temp, "Z:SIGINT received, exiting server Z. Total requests handled %d, %d invertible, %d not.\n", handled, inverted, handled - inverted);
        write(fd, temp, strlen(temp));
    }
    exit(EXIT_FAILURE);
}

void *create_shared_memory2(size_t size)
{
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    return mmap(NULL, size, protection, visibility, -1, 0);
}
void checkArguments(int argc, char **argv, char *pathToServerFifo, char *pathToLogFile, int *poolSize, int *poolSize2, int *durationTime);

struct childStatus
{
    int pid;
    int status;
};
typedef struct childStatus childStatus;

int main(int argc, char **argv, char **argve)
{
    // becomeDaemon(0);
    int shm;
    sem_t *commonSem;
    if ((shm = shm_open("myshm", O_RDWR | O_CREAT, S_IRWXU)) == 0)
    {
        perror("shm_open Z");
        exit(1);
    }
    if (ftruncate(shm, sizeof(sem_t)) < 0)
    {
        perror("ftruncate Z");
        exit(1);
    }
    if ((commonSem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    sigintSemaphore = mmap(NULL, sizeof(*sigintSemaphore), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sigintSemaphore == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(sigintSemaphore, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    last = mmap(NULL, sizeof(*last), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (last == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(last, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    int clientFd;
    char pathToServerFifo[50], pathToLogFile[50];
    char temp[100];
    request serverZreq;
    int pid = 1;
    request *shmem;
    time_t t;
    childStatus *sharedArray;
    int poolSize = -1, poolSize2 = -1, durationTime = -1;
    checkArguments(argc, argv, pathToServerFifo, pathToLogFile, &poolSize, &poolSize2, &durationTime);
    fd = open(pathToLogFile, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        perror("Open");
        exit(EXIT_FAILURE);
    }
    time(&t);
    sprintf(temp, "%sZ output:\n", ctime(&t));
    write(fd, temp, strlen(temp));
    time(&t);
    sprintf(temp, "%s(Z:Server Z %s, t%d, r = %d) started\n", ctime(&t), pathToLogFile, durationTime, poolSize2);
    write(fd, temp, strlen(temp));
    handledSemaphore = mmap(NULL, sizeof(*handledSemaphore), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (handledSemaphore == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(handledSemaphore, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    invertedSemaphore = mmap(NULL, sizeof(*invertedSemaphore), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (invertedSemaphore == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(invertedSemaphore, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    busy = mmap(NULL, sizeof(*busy), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (busy == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(busy, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    shmem = create_shared_memory(128);
    sharedArray = (childStatus *)create_shared_memory2(128);
    childStatus *childArray = (childStatus *)malloc(sizeof(childStatus) * poolSize2);
    for (int i = 0; i < poolSize2; i++)
    {
        if (pid != 0)
        {
            pid = fork();
            if (pid != 0)
            {
                childArray[i].pid = pid;
                childArray[i].status = 0;
                kill(pid, SIGSTOP);
            }
        }
    }
    int getValue;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    if (pid != 0)
    {
        memcpy(sharedArray, childArray, sizeof(childArray) * poolSize2);
    }
    free(childArray);
    if (pid != 0)
    {
        for (;;)
        {

            sem_getvalue(last, &getValue);
            if (getValue < poolSize2)
            {
                if (read(atoi(argve[0]), &serverZreq, sizeof(serverZreq)) == sizeof(serverZreq))
                {
                    sem_post(commonSem);
                    memcpy(shmem, &serverZreq, sizeof(serverZreq));
                    for (int i = 0; i < poolSize2; i++)
                    {

                        if (sharedArray[i].status == 0)
                        {
                            sem_post(last);
                            sharedArray[i].status = 1;
                            kill(sharedArray[i].pid, SIGCONT);
                            break;
                        }
                    }
                }
            }
            else
            {
                while (getValue >= poolSize2)
                {
                    sem_getvalue(last, &getValue);
                }
            }
        }
    }
    else
    {
        for (;;)
        {
            for (int i = 0; i < poolSize2; i++)
            {
                if (getpid() == sharedArray[i].pid)
                {

                    sem_post(busy);
                    int a;
                    sem_getvalue(busy, &a);
                    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO, (int)shmem->pid);
                    clientFd = open(clientFifo, O_WRONLY);
                    if (clientFd == -1)
                    {
                        perror("clientFd");
                    }
                    time(&t);
                    sprintf(temp, "%sZ:Worker PID#%d, is handling client PID# %d, matrix size %dx%x, pool busy %d/%d\n", ctime(&t), getpid(), shmem->pid, getMatrixSize(shmem->matrix), getMatrixSize(shmem->matrix), a, poolSize2);
                    write(fd, temp, strlen(temp));
                    int flag = 1;
                    sleep(durationTime);
                    // isInvertible(matrix, getMatrixSize(child_request.matrix));
                    if (flag)
                    {
                        sem_post(invertedSemaphore);
                        time(&t);
                        sprintf(temp, "%sZ:Worker PID#%d responding to client PID#%d: the matrix IS invertible.\n", ctime(&t), getpid(), shmem->pid);
                        write(fd, temp, strlen(temp));
                        write(clientFd, "samet", strlen("samet"));
                        if (close(clientFd) == -1)
                        {
                            perror("close");
                        }
                    }
                    else
                    {
                        time(&t);
                        sprintf(temp, "%sZ:Worker PID#%d responding to client PID#%d: the matrix IS NOT invertible.\n", ctime(&t), getpid(), shmem->pid);
                        write(fd, temp, strlen(temp));
                        write(clientFd, "samet", strlen("samet"));
                        if (close(clientFd) == -1)
                        {
                            perror("close");
                        }
                    }
                    sharedArray[i].status = 0;
                    sem_wait(busy);
                    sem_wait(commonSem);
                    sem_post(handledSemaphore);
                    sem_wait(last);
                    kill(getpid(), SIGSTOP);
                }
            }
        }
    }
    return 0;
}
void checkArguments(int argc, char **argv, char *pathToServerFifo, char *pathToLogFile, int *poolSize, int *poolSize2, int *durationTime)
{
    int option;
    if (argc != 11)
    {
        perror("Invalid Argument Number!\n");
        exit(EXIT_FAILURE);
    }
    while ((option = getopt(argc, argv, "p:r:t:s:o:")) != -1)
    {
        switch (option)
        {
        case 's':
            strcpy(pathToServerFifo, optarg);
            break;
        case 'p':
            (*poolSize) = atoi(optarg);
            break;
        case 'o':
            strcpy(pathToLogFile, optarg);
            break;

        case 'r':
            (*poolSize2) = atoi(optarg);
            break;
        case 't':
            (*durationTime) = atoi(optarg);
            break;
        }
    }
    if (pathToServerFifo == NULL || pathToLogFile == NULL || *poolSize == -1 || *poolSize2 == -1 || *durationTime == -1)
    {
        perror("Invalid Arguments");
        exit(EXIT_FAILURE);
    }
}
void getCofactor(int **matrix, int **temp, int p, int q, int n)
{
    int i = 0, j = 0;
    for (int row = 0; row < n; row++)
    {
        for (int col = 0; col < n; col++)
        {
            if (row != p && col != q)
            {
                temp[i][j++] = matrix[row][col];
                if (j == n - 1)
                {
                    j = 0;
                    i++;
                }
            }
        }
    }
}
int determinantOfMatrix(int **matrix, int n)
{
    int D = 0;
    if (n == 1)
        return matrix[0][0];
    int **temp;
    temp = (int **)malloc(n * sizeof(int));
    for (int i = 0; i < n; ++i)
        temp[i] = (int *)malloc(n * sizeof(int));
    int sign = 1;
    for (int f = 0; f < n; f++)
    {
        getCofactor(matrix, temp, 0, f, n);
        D += sign * temp[0][f] * determinantOfMatrix(temp, n - 1);
        sign = -sign;
    }
    return D;
}
int isInvertible(int **matrix, int n)
{
    if (determinantOfMatrix(matrix, n) != 0)
        return 1;
    else
        return 0;
}
int getMatrixSize(char *charMatrix)
{
    int comma = 0, newLine = 0;
    for (int i = 0; i < strlen(charMatrix); i++)
    {
        if (charMatrix[i] == ',')
        {
            comma++;
        }
        else if (charMatrix[i] == '\n')
        {
            newLine++;
        }
    }
    newLine++;
    return newLine;
    // return (newLine++ == comma / newLine) ? 1 : 0;
}
int /* Returns 0 on success, -1 on error */
becomeDaemon(int flags)
{
    int maxfd, fd2;

    switch (fork())
    { /* Become background process */
    case -1:
        return -1;
    case 0:
        break; /* Child falls through... */
    default:
        _exit(EXIT_SUCCESS); /* while parent terminates */
    }

    if (setsid() == -1) /* Become leader of new session */
        return -1;

    switch (fork())
    { /* Ensure we are not session leader */
    case -1:
        return -1;
    case 0:
        break;
    default:
        _exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))
        umask(0); /* Clear file mode creation mask */

    if (!(flags & BD_NO_CLOSE_FILES))
    { /* Close all open files */
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1)          /* Limit is indeterminate... */
            maxfd = BD_MAX_CLOSE; /* so take a guess */

        for (fd2 = 0; fd2 < maxfd; fd2++)
            close(fd2);
    }

    if (!(flags & BD_NO_REOPEN_STD_FDS))
    {
        close(STDIN_FILENO); /* Reopen standard fd's to /dev/null */

        fd2 = open("/dev/null", O_RDWR);

        if (fd2 != STDIN_FILENO) /* 'fd' should be 0 */
            return -1;
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -1;
    }

    return 0;
}