#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include "fifo_seqnum.h"

#define CLIENT_FIFO "/tmp/client%d"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO) + 20)
#define FIFO_PERM (S_IRUSR | S_IWUSR | S_IWGRP)

static char clientFifo[CLIENT_FIFO_NAME_LEN];
char logFile[100];
static void removeFifo(void)
{
    unlink(clientFifo);
}

extern char *optarg;
int getMatrixSize(char *charMatrix);
void checkArguments(int argc, char *argv[]);
void readFile(char *filePath, char *pathToServerFifo);
void createFifo(request client_request, char *pathToServerFifo);
int main(int argc, char **argv)
{
    checkArguments(argc, argv);
    // readFile("test.csv");
    //  createFifo(array);

    return 0;
}
void checkArguments(int argc, char *argv[])
{
    char pathToServerFifo[100], pathToDataFile[100];
    int option;
    if (argc != 5)
    {
        perror("Invalid Argument Number!\n");
        return;
    }
    while ((option = getopt(argc, argv, "s:o:")) != -1)
    {
        switch (option)
        {
        case 's':
            // pathToServerFifo = (char *)malloc(strlen(optarg));
            strcpy(pathToServerFifo, optarg);
            break;
        case 'o':
            // pathToDataFile = (char *)malloc(strlen(optarg));
            strcpy(pathToDataFile, optarg);
            break;
        default: /* '?' */
            perror("Invalid Arguments");
            return;
        }
    }
    if (pathToServerFifo == NULL || pathToDataFile == NULL)
    {
        perror("Invalid Arguments");
        return;
    }
    readFile(pathToDataFile, pathToServerFifo);
}

void readFile(char *filePath, char *pathToServerFifo)
{

    int fd = open(filePath, O_RDONLY);
    if (fd == -1)
    {
        perror("Open File");
    }
    int sz = 1;
    char ch;
    int count = 0;
    int column = 0;
    char *matrix; // default matrix size
    if (fd == -1)
    {
        perror("open");
    }
    while (sz > 0)
    {
        count++;
        sz = read(fd, &ch, 1);
    }
    count++;
    matrix = (char *)malloc(sizeof(char) * count);
    lseek(fd, 0, SEEK_SET);
    sz = 1;
    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        matrix[column] = ch;
        column++;
    }
    matrix[column - 1] = '\0';
    request client_request;
    strcpy(client_request.matrix, matrix);
    client_request.pid = getpid();
    strcpy(logFile, filePath);
    free(matrix);
    close(fd);
    createFifo(client_request, pathToServerFifo);
}
void createFifo(request client_request, char *pathToServerFifo)
{
    clock_t start;
    /*Do something*/
    clock_t end;
    time_t t;
    int serverFd, clientFd;
    char clientResponse[100];
    char temp[200];
    umask(0);
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO, (int)getpid());
    if (mkfifo(clientFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        exit(0);
    }
    if (atexit(removeFifo) != 0)
    {
        perror("atexit");
        exit(0);
    }
    serverFd = open(pathToServerFifo, O_WRONLY);
    if (serverFd == -1)
    {
        perror("Open fifo");
        exit(EXIT_FAILURE);
    }
    time(&t);
    start = clock();
    sprintf(temp, "%sClient PID#%d (%s) is submitting a %dx%d matrix\n", ctime(&t), getpid(), logFile, getMatrixSize(client_request.matrix), getMatrixSize(client_request.matrix));
    write(1, temp, strlen(temp));

    write(serverFd, &client_request, sizeof(client_request));
    clientFd = open(clientFifo, O_RDONLY);
    if (clientFd == -1)
        perror("clientFifo");
    read(clientFd, clientResponse, 100);
    end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    if (strcmp(clientResponse, "true") == 0)
    {
        time(&t);
        sprintf(temp, "%sClient PID#%d: the matrix is invertible, total time %f seconds, goodbye.\n", ctime(&t), getpid(), seconds);
        write(1, temp, strlen(temp));
    }
    else
    {
        time(&t);
        sprintf(temp, "%sClient PID#%d: the matrix is invertible, total time %f seconds, goodbye.\n", ctime(&t), getpid(), seconds);
        write(1, temp, strlen(temp));
    }
    close(serverFd);
    close(clientFd);
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