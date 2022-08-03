#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
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
#include <sys/socket.h>
#include <arpa/inet.h>

#include "client.h"

int sig = 0;
int requestSize;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int num_threads = 0, totalRequest = 0;
int flag = 0;
pthread_t *threadArray;
request *requestList;
int *indexArray;
int port;
char ipAddr[CHAR_ARRAY_SIZE];

void handler(int signalNum) // sigint kontrolleri yapılmalı
{
    if (signalNum == SIGINT)
    {
        sig = 1;
    }
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    void *res;
    char inputFile[CHAR_ARRAY_SIZE];
    checkArguments(argc, argv, inputFile, ipAddr, &port); // argument control
    readFile(inputFile, &requestList, &requestSize);      // file reading
    /*
        Thread creation
    */
    printTime();
    printf("Client: I have loaded %d requests and I’m creating %d threads.\n", requestSize, requestSize);

    threadArray = (pthread_t *)malloc(sizeof(pthread_t) * requestSize);
    indexArray = (int *)malloc(sizeof(int) * requestSize);

    for (int i = 0; i < requestSize; i++)
    {
        indexArray[i] = i;
        if (pthread_create(&threadArray[i], NULL, requestHandler, &indexArray[i]))
        {
            fprintf(stderr, "Error creating thread\n");
            clearResources();
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < requestSize; i++)
    {
        if (sig)
        {
            clearResources();
            exit(EXIT_FAILURE);
        }
        if (pthread_join(threadArray[i], &res))
        {
            fprintf(stderr, "Error joining thread\n");
            clearResources();
            exit(EXIT_FAILURE);
        }
    }

    clearResources();
    printTime();
    printf("All threads have terminated, goodbye.\n");
    return 0;
}

void checkArguments(int argc, char **argv, char *inputFile, char *ipAdrr, int *port)
{
    int option;
    if (argc != 7)
    {
        fprintf(stderr, "Invalid argument number\n");
        clearResources();
        exit(EXIT_FAILURE);
    }
    while ((option = getopt(argc, argv, "r:s:q:")) != -1)
    {
        switch (option)
        {
        case 'r':
            strcpy(inputFile, optarg);
            break;
        case 's':
            strcpy(ipAdrr, optarg);
            break;
        case 'q':
            *port = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Invalid arguments\n");
            clearResources();
            exit(EXIT_FAILURE);
        }
    }
    if (inputFile == NULL || ipAdrr == NULL)
    {
        fprintf(stderr, "Invalid arguments\n");
        clearResources();
        exit(EXIT_FAILURE);
    }
}
void readFile(char *inputFile, request **requestList, int *requestSize)
{
    int fd, sz = 1, index = 0;
    char ch, *line;
    request temp;
    int defaultLineSize = ARRAY_SIZE;
    line = (char *)malloc(sizeof(char) * defaultLineSize);
    (*requestList) = (request *)malloc(sizeof(request) * 1);
    fd = open(inputFile, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        fprintf(stderr, "File couldn't found\n");
        free(line);
        clearResources();
        exit(EXIT_FAILURE);
    }
    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        if (index == defaultLineSize - 1)
        {
            defaultLineSize = defaultLineSize * 2;
            line = (char *)realloc(line, defaultLineSize);
        }
        if (ch == '\n')
        {
            line[index] = '\0';
            if (parseLine(line, &temp) == 0)
            {
                index = 0;
                continue;
            }
            else
            {

                if (*requestSize != 0)
                {

                    (*requestList) = (request *)realloc((*requestList), sizeof(request) * ((*requestSize) + 1));
                }
                (*requestList)[*requestSize] = temp;
                *requestSize = *requestSize + 1;
            }
            index = 0;
        }
        else
        {
            line[index] = ch;
            index++;
        }
    }
    line[index - 1] = '\0';
    if (parseLine(line, &temp))
    {
        (*requestList) = (request *)realloc((*requestList), sizeof(request) * ((*requestSize) + 1));
        (*requestList)[*requestSize] = temp;
        *requestSize = *requestSize + 1;
    }
    free(line);
    close(fd);
}

int parseLine(char *line, request *req)
{
    char *token;
    int argnum = 0;
    // if the request consist on more than 4 words, it is invalid
    argnum = getArgNum(line);
    if (argnum < 2)
    {
        return 0;
    }
    token = strtok(line, " ");
    int count = 0;
    while (token != NULL)
    {
        switch (count)
        {
        case 0:
            strcpy(req->transcationCount, token);
            break;
        case 1:
            strcpy(req->field_type, token);
            break;
        case 2:

            if (!parseDate(token, &req->start_date))
            {
                return 0;
            }
            break;
        case 3:
            if (!parseDate(token, &req->end_date))
            {
                return 0;
            }
            break;
        case 4:
            strcpy(req->city, token);
        default:
            break;
        }
        count++;
        token = strtok(NULL, " ");
    }
    if (count == 4)
    {
        strcpy(req->city, "ALL");
    }
    return 1;
}

void *requestHandler(void *arg)
{
    int socket_fd = 0, client_fd = 0;
    struct sockaddr_in server_addr;
    int index = *((int *)arg);
    printTime();
    printf("Client-Thread-%d: Thread-%d has been created\n", index, index);
    pthread_mutex_lock(&mutex);
    ++num_threads;
    while (num_threads < requestSize)
    {
        if (pthread_cond_wait(&cond, &mutex) != 0)
        {
            fprintf(stderr, "Error thread condition waiting\n");
            clearResources();
            exit(EXIT_FAILURE);
        }
    }
    if (flag == 0)
    {
        if (pthread_cond_broadcast(&cond) != 0)
        {
            fprintf(stderr, "Error thread condition broadcasting\n");
            clearResources();
            exit(EXIT_FAILURE);
        }
        flag = 1;
    }
    pthread_mutex_unlock(&mutex);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "Error creating socket\n");
        clearResources();
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ipAddr);
    client_fd = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (client_fd < 0)
    {
        if (sig)
        {
            clearResources();
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Error connecting to server\n");
        clearResources();
        exit(EXIT_FAILURE);
    }

    message temp;
    temp.type = CLIENT_REQUEST;
    memcpy(temp.buffer, (void *)&requestList[*(int *)arg], sizeof(request));
    printTime();
    printf("Client-Thread-%d: I am requesting ", index);
    printRequest(requestList[index]);
    printf("\n");
    if (write(socket_fd, (void *)&temp, sizeof(temp)) == -1)
    {
        fprintf(stderr, "Error writing to socket\n");
        clearResources();
        exit(EXIT_FAILURE);
    }
    message response;
    int readControl = read(socket_fd, &response, sizeof(message));
    if (readControl < 0)
    {
        if (sig)
        {
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Error reading from socket\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printTime();
        printf("Client-Thread-%d: The server's response to ", index);
        printRequest(requestList[index]);
        if (response.type == NO_CITY_RESPONSE)
        {
            printf(" is 0\n");
        }
        else
        {
            printf(" is %d", atoi((char *)response.buffer));
            printf("\n");
        }
    }
    close(socket_fd);
    printTime();
    printf("Client-Thread-%d: terminating\n", index);
    return NULL;
}
void clearResources()
{
    if (requestList != NULL)
    {
        free(requestList);
    }
    if (threadArray != NULL)
    {
        free(threadArray);
    }
    if (indexArray != NULL)
    {
        free(indexArray);
    }
    requestList = NULL;
    threadArray = NULL;
    indexArray = NULL;
}