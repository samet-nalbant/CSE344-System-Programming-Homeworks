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
#include "fifo_seqnum.h"
#include <sys/wait.h>
#include <semaphore.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>
#include "become_daemon.h"

#define CLIENT_FIFO "/tmp/client%d"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO) + 20)
int isInvertible(int **matrix, int n);
int determinantOfMatrix(int **matrix, int n);
void getCofactor(int **matrix, int **temp, int p, int q, int n);
int getMatrixSize(char *charMatrix);
int serverZpid = -1;
sem_t *sigintSemaphore;
sem_t *handledSemaphore;
sem_t *forwardedSemaphore;
sem_t *invertedSemaphore;
int fd;
extern char *optarg;
void handler(int sig_num)
{
    sem_post(sigintSemaphore);
    int count;
    char temp[150];
    sem_getvalue(sigintSemaphore, &count);
    if (count == 1)
    {

        int handled, forwarded, inverted;
        sem_getvalue(handledSemaphore, &handled);
        sem_getvalue(forwardedSemaphore, &forwarded);
        sem_getvalue(invertedSemaphore, &inverted);
        sprintf(temp, "SIGINT received, terminating Z and exiting server Y. Total requests handled: %d, %d invertible, %d not. %d requests were forwarded.\n", handled, inverted, handled - inverted, forwarded);
        write(fd, temp, strlen(temp));
        remove("double_instantiation");
    }
    if (serverZpid != -1)
    {

        kill(serverZpid, SIGINT);
    }
    exit(0);
}
void checkArguments(int argc, char **argv, char *pathToServerFifo, char *pathToLogFile, int *poolSize, int *poolSize2, int *durationTime);

int main(int argc, char **argv)
{

    // becomeDaemon(0);
    int tempfd = open("double_instantiation", O_WRONLY);
    if (tempfd == -1)
    {
        open("double_instantiation", O_CREAT);
    }
    else
    {
        perror("ServerY is already exist\n");
        exit(EXIT_FAILURE);
    }
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    int serverFd, dummyFd, clientFd;
    request client_request, child_request;
    char pathToServerFifo[20], pathToLogFile[20];
    char temp[100];
    time_t t;
    int poolSize = -1, poolSize2 = -1, durationTime = -1;
    checkArguments(argc, argv, pathToServerFifo, pathToLogFile, &poolSize, &poolSize2, &durationTime);
    fd = open(pathToLogFile, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        perror("Open");
        exit(EXIT_FAILURE);
    }
    int shm;
    sem_t *sharedSema;
    if ((shm = shm_open("myshm", O_RDWR | O_CREAT, S_IRWXU)) == 0)
    {
        perror("shm_open");
        exit(1);
    }
    if (ftruncate(shm, sizeof(sem_t)) < 0)
    {
        perror("ftruncate");
        exit(1);
    }
    if ((sharedSema = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    if (sem_init(sharedSema, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    sem_t *sema = mmap(NULL, sizeof(*sema), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sema == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(sema, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
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
    forwardedSemaphore = mmap(NULL, sizeof(*forwardedSemaphore), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (forwardedSemaphore == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(forwardedSemaphore, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    invertedSemaphore = mmap(NULL, sizeof(*forwardedSemaphore), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
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

    int pid;
    int serverP[2];
    if (pipe(serverP) < 0)
        exit(1);
    if ((pid = fork()) == -1)
    {
        perror("fork error");
    }
    else if (pid > 0)
    {
        serverZpid = pid;
    }
    else if (pid == 0)
    {

        char **env = malloc(sizeof(char *));
        env[0] = malloc(sizeof(char) * 10);
        sprintf(env[0], "%d", serverP[0]);
        execve("./serverZ", argv, env);
    }

    time(&t);
    sprintf(temp, "%s(Server Y %s, p = %d, t = %d) started\n", ctime(&t), pathToLogFile, poolSize, durationTime);
    write(fd, temp, strlen(temp));
    time(&t);
    sprintf(temp, "%sInstantiated server Z\n", ctime(&t));
    write(fd, temp, strlen(temp));
    pid = 1;
    int p[2];
    umask(0);
    if (mkfifo(pathToServerFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    serverFd = open(pathToServerFifo, O_RDONLY);
    if (serverFd == -1)
        perror("serverFd");
    dummyFd = open(pathToServerFifo, O_WRONLY);
    if (dummyFd == -1)
        perror("dummyFd");
    if (pipe(p) < 0)
        exit(1);

    for (int i = 0; i < poolSize; i++)
    {
        if (pid != 0)
        {
            pid = fork();
        }
    }
    if (pid != 0)
    {
        for (;;)
        {
            int a, b;
            sem_getvalue(sema, &a);
            sem_getvalue(sharedSema, &b);
            if (a >= poolSize && b >= poolSize2)
            {
                sem_getvalue(sema, &a);
                sem_getvalue(sharedSema, &b);
            }
            else
            {
                read(serverFd, &client_request, sizeof(client_request));
                if (a < poolSize)
                {
                    sem_post(sema);
                    write(p[1], &client_request, sizeof(client_request));
                }
                else if (a >= poolSize && b < poolSize2)
                {
                    sem_post(forwardedSemaphore);
                    time(&t);
                    write(serverP[1], &client_request, sizeof(client_request));
                    sprintf(temp, "%sForwarding request of client PID#%d to serverZ, matrix size %dx%d, pool busy %d/%d\n", ctime(&t), client_request.pid, getMatrixSize(client_request.matrix), getMatrixSize(client_request.matrix), a, a);
                    write(fd, temp, strlen(temp));
                }
            }
        }
    }
    else
    {
        for (;;)
        {

            if (read(p[0], &child_request, sizeof(child_request)) == sizeof(struct request))
            {
                int busy;
                sem_getvalue(sema, &busy);
                time(&t);
                sprintf(temp, "%sWorker PID#%d, is handling client PID# %d, matrix size %dx%x, pool busy %d/%d\n", ctime(&t), getpid(), child_request.pid, getMatrixSize(child_request.matrix), getMatrixSize(child_request.matrix), busy, poolSize);
                write(fd, temp, strlen(temp));
                snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO, (int)child_request.pid);
                clientFd = open(clientFifo, O_WRONLY);

                if (clientFd == -1)
                {
                    perror("clientFd1");
                }

                int flag = 1;
                sleep(durationTime);
                // isInvertible(matrix, getMatrixSize(child_request.matrix));
                if (flag)
                {
                    // sem_post(invertedSemaphore);
                    time(&t);
                    sprintf(temp, "%sWorker PID#%d responding to client PID#%d: the matrix IS invertible.\n", ctime(&t), getpid(), child_request.pid);
                    write(fd, temp, strlen(temp));
                }
                else
                {
                    time(&t);
                    sprintf(temp, "%sWorker PID#%d responding to client PID#%d: the matrix IS NOT invertible.\n", ctime(&t), getpid(), child_request.pid);
                    write(fd, temp, strlen(temp));
                }
                // sleep(durationTime);
                sem_post(handledSemaphore);
                write(clientFd, "samet", strlen("samet"));
                if (close(clientFd) == -1)
                {
                    perror("close");
                }
            }
            sem_wait(sema);
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
