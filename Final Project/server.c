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
#include <sys/un.h>
#include "server.h"

pthread_t *threadArray;
int *indexArray;
int *statusArray;
int sig = 0;
int count = 0;
int *fd_array;
struct Queue *requestQueue;
portItem *portList;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int totalHandledNumber = 0;
int totalPortNumber = 0;
int portListSize = ARRAY_SIZE;

void handler(int signalNum)
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
    int fd_array_size = 100;
    fd_array = malloc(sizeof(int) * fd_array_size);
    int fd_count = 0;
    requestQueue = createQueue(100);
    portList = (portItem *)malloc(sizeof(portItem) * portListSize);
    int port, numThreads, addr_len;
    checkArguments(argc, argv, &port, &numThreads);

    /*
        Thrread creation
    */
    threadArray = (pthread_t *)malloc(sizeof(pthread_t) * numThreads);
    indexArray = (int *)malloc(sizeof(int) * numThreads);
    statusArray = (int *)malloc(sizeof(int) * numThreads);

    for (int i = 0; i < numThreads; i++)
    {
        pthread_attr_t detachedThread;
        pthread_attr_init(&detachedThread);
        pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);
        indexArray[i] = i;
        statusArray[i] = FREE;
        if (pthread_create(&threadArray[i], &detachedThread, serverHandler, &indexArray[i]))
        {
            fprintf(stderr, "Error creating thread\n");
            exit(EXIT_FAILURE);
        }
    }

    /*
        Socket Creation
    */
    struct sockaddr_in my_addr;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == 0)
    {
        fprintf(stderr, "Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);
    addr_len = sizeof(my_addr);
    const int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n");
    if (bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        fprintf(stderr, "Socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    if (listen(socket_fd, 5) < 0)
    {
        fprintf(stderr, "Listen failed\n");
        exit(EXIT_FAILURE);
    }
    int totalConnection = 0;
    for (;;)
    {
        if (fd_count == fd_array_size - 1)
        {
            fd_array_size *= 2;
            fd_array = realloc(fd_array, sizeof(int) * fd_array_size);
        }

        fd_array[fd_count] = accept(socket_fd, (struct sockaddr *)&my_addr, (socklen_t *)&addr_len);
        if (fd_array[fd_count] < 0)
        {
            if (sig)
            {
                printf("SIGINT received\n");
                handleSigint();
                free_serverResources();
                printf("SIGINT has been received. I handled a total of %d requests. Goodbye.\n", totalHandledNumber);
                exit(EXIT_SUCCESS);
            }
            fprintf(stderr, "Request accept failed\n");
            exit(EXIT_FAILURE);
        }
        totalConnection++;
        enqueue(requestQueue, fd_array[fd_count]);
        fd_count++;
        for (int i = 0; i < numThreads; i++)
        {
            if (statusArray[i] == FREE)
            {
                pthread_cond_signal(&cond);
                break;
            }
        }
    }

    close(socket_fd);
    return 0;
}
void checkArguments(int argc, char **argv, int *port, int *numThreads)
{
    int option;
    if (argc != 5)
    {
        fprintf(stderr, "Invalid argument number\n");
        exit(EXIT_FAILURE);
    }
    while ((option = getopt(argc, argv, "p:t:")) != -1)
    {
        switch (option)
        {
        case 'p':
            *port = atoi(optarg);
            break;
        case 't':
            *numThreads = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Invalid Arguments \n");
            exit(EXIT_FAILURE);
        }
    }
    if (*numThreads < 5)
    {
        fprintf(stderr, "Number of threads must be greater than 5\n");
        exit(EXIT_FAILURE);
    }
}

void *serverHandler(void *arg)
{
    int read_control;
    message temp;
    for (;;)
    {
        if (sig)
        {
            exit(EXIT_FAILURE);
        }
        if (!isEmpty(requestQueue))
        {
            pthread_mutex_lock(&mutex);
            count++;
            int new_socket_fd = dequeue(requestQueue);
            if (new_socket_fd != -1 && statusArray[*(int *)arg] == FREE)
            {
                pthread_mutex_unlock(&mutex);
                statusArray[*(int *)arg] = IN_USE;
                read_control = read(new_socket_fd, &temp, sizeof(temp));
                if (read_control < 0)
                {
                    fprintf(stderr, "Request read failed\n");
                    exit(EXIT_FAILURE);
                }
                if (temp.type == CLIENT_REQUEST)
                {
                    request *req = (request *)temp.buffer;
                    printTime();
                    printf("Request arrived ");
                    printRequest(*req);
                    printf("\n");
                    int allTransactions = 0;
                    if (strcmp(req->city, "ALL") == 0)
                    {
                        printTime();
                        for (int i = 0; i < totalPortNumber; i++)
                        {
                            int socket_fd2 = 0, servant_fd = 0;
                            struct sockaddr_in server_addr;
                            socket_fd2 = socket(AF_INET, SOCK_STREAM, 0);
                            if (socket_fd2 < 0)
                            {
                                fprintf(stderr, "Socket creation failed\n");
                                exit(EXIT_FAILURE);
                            }
                            server_addr.sin_family = AF_INET;
                            server_addr.sin_addr.s_addr = INADDR_ANY;
                            server_addr.sin_port = htons(portList[i].port);
                            servant_fd = connect(socket_fd2, (struct sockaddr *)&server_addr, sizeof(server_addr));
                            if (servant_fd < 0)
                            {
                                fprintf(stderr, "Connection failed\n");
                                exit(EXIT_FAILURE);
                            }
                            if (write(socket_fd2, (void *)&temp, sizeof(temp)) < 0)
                            {
                                fprintf(stderr, "Write error\n");
                            }

                            message response;
                            read_control = read(socket_fd2, &response, sizeof(message));
                            if (read_control < 0)
                            {
                                fprintf(stderr, "Request read failed\n");
                                close(socket_fd2);
                            }
                            allTransactions += atoi((char *)response.buffer);
                            close(socket_fd2);
                        }
                        message results;
                        results.type = RESPONSE;
                        sprintf((char *)results.buffer, "%d", allTransactions);
                        printTime();
                        printf("Response received: %d, forwarded to client\n", allTransactions);
                        if (write(new_socket_fd, &results, sizeof(message)) < 0)
                        {
                            fprintf(stderr, "Write error\n");
                        }
                        totalHandledNumber++;
                        close(new_socket_fd);
                    }
                    else
                    {
                        portItem tempPort = findPortNumber(req->city);
                        int servantPort = tempPort.port;
                        if (servantPort == -1)
                        {
                            message response;
                            response.type = NO_CITY_RESPONSE;
                            sprintf((char *)response.buffer, "%d", -1);
                            printTime();
                            printf("Proper servant for %s is couldn't found, responsing to to client\n", req->city);
                            if (write(new_socket_fd, &response, sizeof(message)) < 0)
                            {
                                fprintf(stderr, "? Error in writing to client\n");
                            }
                            totalHandledNumber++;
                            close(new_socket_fd);
                        }
                        else
                        {
                            int socket_fd2 = 0, servant_fd = 0;
                            struct sockaddr_in server_addr;
                            /*
                            Socket Creation
                            */
                            socket_fd2 = socket(AF_INET, SOCK_STREAM, 0);
                            if (socket_fd2 < 0)
                            {
                                fprintf(stderr, "Socket creation error\n");
                                exit(EXIT_FAILURE);
                            }
                            server_addr.sin_family = AF_INET;
                            server_addr.sin_port = htons(servantPort);
                            server_addr.sin_addr.s_addr = INADDR_ANY;
                            printTime();
                            printf("Contacting servant %d\n", tempPort.processID);
                            servant_fd = connect(socket_fd2, (struct sockaddr *)&server_addr, sizeof(server_addr));
                            if (servant_fd < 0)
                            {
                                fprintf(stderr, "Connection error\n");
                                exit(EXIT_FAILURE);
                            }
                            if (write(socket_fd2, (void *)&temp, sizeof(temp)) < 0)
                            {
                                fprintf(stderr, "Write error\n");
                            }

                            message response;
                            read_control = read(socket_fd2, &response, sizeof(message));
                            if (read_control < 0)
                            {
                                fprintf(stderr, "Read error\n");
                                close(socket_fd2);
                            }
                            else
                            {
                                printTime();
                                printf("Response received: %d, forwarded to client\n", atoi((char *)response.buffer));
                                if (write(new_socket_fd, &response, sizeof(message)) < 0)
                                {
                                    fprintf(stderr, "Write error\n");
                                }
                                totalHandledNumber++;
                                close(new_socket_fd);
                            }
                            close(socket_fd2);
                        }
                    }
                }
                else if (temp.type == SERVANT_DECLARATION)
                {
                    pthread_mutex_lock(&fileMutex);
                    servantDeclaration *servant_req = (servantDeclaration *)temp.buffer;
                    addToPortList(servant_req);
                    printTime();
                    printf("Servant %d present at port %d handling cities %s - %s\n", servant_req->processID, servant_req->port, servant_req->cities[0], servant_req->cities[servant_req->total_city_number - 1]);
                    close(new_socket_fd);
                    pthread_mutex_unlock(&fileMutex);
                }
                else
                {
                    fprintf(stderr, "Uncrecognized request\n");
                }
            }
            else
            {
                pthread_mutex_unlock(&mutex);
            }
            statusArray[*(int *)arg] = FREE;
        }
        else
        {
            pthread_mutex_lock(&mutex);
            if (pthread_cond_wait(&cond, &mutex) != 0)
            {
                fprintf(stderr, "%s", "Condition wait error\n");
                exit(EXIT_FAILURE);
            }
            pthread_mutex_unlock(&mutex);
        }
    }

    return NULL;
}

void addToPortList(servantDeclaration *item)
{
    if (totalPortNumber == portListSize - 1)
    {
        portListSize *= 2;
        portList = realloc(portList, sizeof(portItem) * portListSize);
    }
    portList[totalPortNumber].port = item->port;
    portList[totalPortNumber].processID = item->processID;
    int index = 0;
    for (int i = 0; i < item->total_city_number; i++)
    {
        strcpy(portList[totalPortNumber].cities[index], item->cities[i]);
        index++;
    }
    portList[totalPortNumber].city_count = index;
    totalPortNumber++;
}

portItem findPortNumber(char *city)
{
    for (int i = 0; i < totalPortNumber; i++)
    {
        for (int j = 0; j < portList[i].city_count; j++)
        {
            if (strcmp(portList[i].cities[j], city) == 0)
            {
                return portList[i];
            }
        }
    }
    portItem temp;
    temp.port = -1;
    return temp;
}
void handleSigint()
{

    printTime();
    printf("Contacting ALL to deliver SIGINT to servants\n");
    message exitMessage;
    exitMessage.type = EXIT;
    for (int i = 0; i < totalPortNumber; i++)
    {
        int socket_fd2 = 0, servant_fd = 0;
        struct sockaddr_in server_addr;
        socket_fd2 = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd2 < 0)
        {
            fprintf(stderr, "Socket creation failed\n");
            exit(EXIT_FAILURE);
        }
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.2");
        server_addr.sin_port = htons(portList[i].port);
        servant_fd = connect(socket_fd2, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (servant_fd < 0)
        {
            fprintf(stderr, "Connection failed\n");
            exit(EXIT_FAILURE);
        }
        if (write(socket_fd2, (void *)&exitMessage, sizeof(message)) < 0)
        {
            fprintf(stderr, "Write error\n");
        }
    }
}
void free_serverResources()
{
    free(threadArray);
    free(indexArray);
    free(statusArray);
    free(fd_array);
    free(requestQueue->array);
    free(requestQueue);
    free(portList);
}