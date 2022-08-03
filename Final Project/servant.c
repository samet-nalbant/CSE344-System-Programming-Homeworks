#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <math.h>
#include <getopt.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "helper.h"
#include "servant.h"
struct Queue *requestQueue;
int handledRequestNum = 0;
int *fd_array;
int processID;
int sig = 1;
int city_count = 0;
char **cities;
void handler(int signalNum) // sigint kontrolleri yapılmalı
{
    if (signalNum == SIGINT)
    {
        sig = 1;
    }
}

int main(int argc, char *argv[])
{
    char directoryPath[ARRAY_SIZE];
    requestQueue = createQueue(100);
    char ipAddr[CHAR_ARRAY_SIZE];
    char combinedArguments[CHAR_ARRAY_SIZE];
    int start, end, port;
    int fd_array_size = 100;
    fd_array = malloc(sizeof(int) * fd_array_size);
    int fd_count = 0;
    checkArguments(argc, argv, directoryPath, ipAddr, &port, &start, &end, combinedArguments); // argument control
    int emptyPort = findEmptyPort(start);
    int addr_len;
    processID = findProcessNumber(combinedArguments);
    DIR *directory;
    servantDeclaration informations;
    informations.processID = processID;
    cities = (char **)malloc(sizeof(char *) * 1);
    cities[0] = (char *)malloc(sizeof(char) * ARRAY_SIZE);
    struct entry_node *entry_root = NULL;
    struct dirent *dir;
    message temp;
    temp.type = SERVANT_DECLARATION;
    directory = opendir(directoryPath);
    if (directory)
    {
        while ((dir = readdir(directory)) != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                continue;
            }
            else
            {
                strcpy(cities[city_count], dir->d_name);
                cities = (char **)realloc(cities, sizeof(char *) * (city_count + 2));
                cities[city_count + 1] = (char *)malloc(sizeof(char) * ARRAY_SIZE);
                city_count++;
            }
        }
        closedir(directory);
    }
    else
    {
        fprintf(stderr, "Folder couldn't found!\n");
        exit(EXIT_FAILURE);
    }
    if (city_count == 0)
    {
        fprintf(stderr, "No cities found!\n");
        freeResources();
    }
    sortCityNames(cities, city_count);
    informations.total_city_number = end - start + 1;
    informations.port = emptyPort;
    int countIndex = 0;
    if (start > city_count || start < 0)
    {
        fprintf(stderr, "Start city index is out of range!\n");
        freeResources();
    }
    if (end > city_count || end < 0)
    {
        fprintf(stderr, "End city index is out of range!\n");
        freeResources();
    }
    for (int i = start - 1; i < end; i++)
    {
        insertToHash(cities[i]);
        strcpy(informations.cities[countIndex], cities[i]);
        readFile(directoryPath, cities[i], &entry_root);
        countIndex++;
    }
    printTime();
    printf("Servant %d: loaded dataset cities %s - %s\n", processID, informations.cities[0], informations.cities[informations.total_city_number - 1]);
    printTime();
    printf("Servant %d: listening at port %d\n", processID, informations.port);
    int socket_fd = 0, servant_fd = 0;
    struct sockaddr_in server_addr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        fprintf(stderr, "Socket creation failed!\n");
        freeResources();
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ipAddr);
    servant_fd = connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (servant_fd < 0)
    {
        fprintf(stderr, "Connection failed!\n");
        freeResources();
    }
    memcpy(temp.buffer, &informations, sizeof(servantDeclaration));
    send(socket_fd, (char *)&temp, sizeof(message), 0);
    close(socket_fd);
    // close(servant_fd);
    /*
    Sending message to server which port is in used and contacted cities
    */

    /*
    Socket Creation
    */

    /*
    Socket Creation For Transactions
    */
    pthread_attr_t detachedThread;
    pthread_t threadArray[100];
    pthread_attr_init(&detachedThread);
    int count = 0;
    struct sockaddr_in my_addr;
    int socket_fd2 = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd2 < 0)
    {
        fprintf(stderr, "Socket creation failed2\n");
        freeResources();
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(emptyPort);
    addr_len = sizeof(my_addr);
    if (bind(socket_fd2, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        fprintf(stderr, "Socket bind failed\n");
        freeResources();
    }
    if (listen(socket_fd2, 5) < 0)
    {
        fprintf(stderr, "Listen failed\n");
        freeResources();
    }
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
                printf("SIGINT received\n\n\n");
                freeResources();
            }
            fprintf(stderr, "Request accept failed\n");
            freeResources();
        }
        else
        {
            enqueue(requestQueue, fd_array[fd_count]);
            fd_count++;
            pthread_attr_setdetachstate(&detachedThread, PTHREAD_CREATE_DETACHED);
            if (pthread_create(&threadArray[count], &detachedThread, servantHandler, &count))
            {
                fprintf(stderr, "Thread creation error\n");
                freeResources();
            }
            count++;
        }
    }
    close(socket_fd2);
    freeResources();
    return 0;
}
//-d directoryPath -c 10-19 -r IP -p PORT
void checkArguments(int argc, char **argv, char *directoryPath, char *ipAddress, int *port, int *start, int *end, char *combine)
{
    int option;
    if (argc != 9)
    {
        fprintf(stderr, "%s", "Invalid Argument Number\n");
        freeResources();
    }
    while ((option = getopt(argc, argv, "d:c:r:p:")) != -1)
    {
        char *token;
        int count = 0;
        switch (option)
        {
        case 'd':
            strcpy(directoryPath, optarg);
            break;
        case 'c':
            token = strtok(optarg, "-");
            while (token != NULL)
            {
                switch (count)
                {
                case 0:
                    *start = atoi(token);
                    break;
                case 1:
                    *end = atoi(token);
                    break;
                default:
                    break;
                }
                count++;
                token = strtok(NULL, "-");
            }
            if (count != 2)
            {
                fprintf(stderr, "%s", "Invalid Date Argument\n");
                freeResources();
            }
            break;
        case 'r':
            strcpy(ipAddress, optarg);
            break;
        case 'p':
            *port = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s", "Invalid Arguments\n");
            freeResources();
        }
    }
    if (directoryPath == NULL)
    {
        fprintf(stderr, "%s", "Invalid Arguments\n");
        freeResources();
    }
    strcpy(combine, argv[0]);
    strcat(combine, " -d ");
    strncat(combine, directoryPath, strlen(directoryPath));
    strcat(combine, " -c ");
    char temp[CHAR_ARRAY_SIZE];
    sprintf(temp, "%d %d", *start, *end);
    strncat(combine, temp, strlen(temp));
    strcat(combine, " -r ");
    strncat(combine, ipAddress, strlen(ipAddress));
    strcat(combine, " -p ");
    sprintf(temp, "%d", *port);
    strncat(combine, temp, strlen(temp));
}

struct entry_node *createNode(entry *item)
{
    struct entry_node *temp = (struct entry_node *)malloc(sizeof(struct entry_node));
    memcpy(&temp->key, item, sizeof(entry));
    temp->left = temp->right = NULL;
    return temp;
}

void traverse(struct entry_node *root)
{
    if (root != NULL)
    {
        traverse(root->left);
        printDate(root->key.date);
        traverse(root->right);
    }
}
struct entry_node *insert_item(struct entry_node *node, entry *key)
{
    if (node == NULL)
        return createNode(key);

    if (compareDate(key->date, node->key.date) <= 0)
    {
        node->left = insert_item(node->left, key);
    }
    else if (compareDate(key->date, node->key.date) > 0)
        node->right = insert_item(node->right, key);

    return node;
}

void readFile(char *directoryPath, char *cityPath, struct entry_node **entry_root)
{
    DIR *directory;
    struct dirent *dir;
    int fd, sz = 1, index = 0;
    char ch, *line;
    int defaultLineSize = CHAR_ARRAY_SIZE;
    char temp[CHAR_ARRAY_SIZE];
    char temp_directory[CHAR_ARRAY_SIZE];
    char temp2[CHAR_ARRAY_SIZE];
    strcpy(temp, directoryPath);
    int len = strlen(temp);
    temp[len] = '/';
    temp[len + 1] = '\0';
    sprintf(temp + strlen(temp), "%s", cityPath);
    entry temp_entry;
    directory = opendir(temp);
    line = (char *)malloc(sizeof(char) * defaultLineSize);
    if (directory)
    {
        while ((dir = readdir(directory)) != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                continue;
            }
            else
            {
                strcpy(temp2, dir->d_name);
                strcpy(temp_directory, temp);
                if (parseDate(temp2, &temp_entry.date))
                {
                    len = strlen(temp);
                    temp[len] = '/';
                    temp[len + 1] = '\0';
                    sprintf(temp + strlen(temp), "%s", dir->d_name);
                    fd = open(temp, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
                    if (fd == -1)
                    {
                        fprintf(stderr, "%s", "File Open Error\n");
                        freeResources();
                    }
                    index = 0;
                    sz = 1;
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
                            if (parseLine(line, &temp_entry))
                            {
                                addEntry(cityPath, &temp_entry);
                                //*entry_root = insert_item(*entry_root, &temp_entry);
                            }

                            // burda eklencek
                            index = 0;
                        }
                        else
                        {
                            line[index] = ch;
                            index++;
                        }
                    }
                    close(fd);
                }
                strcpy(temp, temp_directory);
            }
        }
        closedir(directory);
    }
    free(line);
}
int parseLine(char *line, entry *item)
{
    char *token;
    int argnum = 0;
    // if the request consist on more than 4 words, it is invalid
    argnum = getArgNum(line);
    if (argnum < 4)
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
            item->transaction_id = atoi(token);
            break;
        case 1:
            strcpy(item->real_estate, token);
            break;
        case 2:
            strcpy(item->street_name, token);
            break;
        case 3:
            item->surface = atoi(token);
            break;
        case 4:
            item->price = atoi(token);
            break;
        default:
            break;
        }
        count++;
        token = strtok(NULL, " ");
    }
    return 1;
}

void sortCityNames(char **cities, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = i + 1; j < size; j++)
        {
            if (strcmp(cities[i], cities[j]) > 0)
            {
                char temp[ARRAY_SIZE];
                strcpy(temp, cities[i]);
                strcpy(cities[i], cities[j]);
                strcpy(cities[j], temp);
            }
        }
    }
}
int isPortExit(int portNumber)
{
    int defaultLineSize = 1024;
    int fd, sz = 1;
    char ch;
    char *line = (char *)malloc(sizeof(char) * defaultLineSize);
    char *token, *token2;
    int lineCount = 0;
    int index = 0;
    fd = open("/proc/net/tcp", O_RDONLY, PROT_READ);
    if (fd == -1)
    {
        fprintf(stderr, "%s", "File Open Error\n");
        freeResources();
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
            int count = 0;
            char *rest = line;
            while ((token = strtok_r(rest, " ", &rest)))
            {
                if (count == 1 && lineCount > 0)
                {
                    char *rest2 = token;
                    int flag = 0;
                    char *end;
                    while ((token2 = strtok_r(rest2, ":", &rest2)))
                    {

                        if (flag)
                        {
                            if ((int)strtol(token2, &end, 16) == portNumber)
                            {
                                return 1;
                            }
                        }
                        flag++;
                    }
                }
                count++;
            }
            index = 0;
            lineCount++;
        }
        else
        {
            line[index] = ch;
            index++;
        }
    }
    close(fd);
    free(line);
    return 0;
}
int findProcessNumber(char *argument)
{
    int fd;
    char ch;
    DIR *directory;
    char file_path[1024];
    int defaultLineSize = 1024;
    char *line = (char *)malloc(sizeof(char) * defaultLineSize);
    int pid;
    int sz = 1;
    struct dirent *dir;
    strcpy(file_path, "/proc/");
    directory = opendir("/proc");
    if (directory)
    {
        while ((dir = readdir(directory)) != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            {
                continue;
            }
            else
            {
                pid = atoi(dir->d_name);
                if (pid)
                {
                    sprintf(file_path + strlen("/proc/"), "%s/cmdline", dir->d_name);
                    fd = open(file_path, O_RDONLY, PROT_READ);
                    sz = 1;
                    if (fd == -1)
                    {
                        fprintf(stderr, "Arguments file of process couldn't opened\n");
                        free(line);
                        freeResources();
                    }
                    int index = 0;
                    while (sz > 0)
                    {
                        if (index == defaultLineSize - 1)
                        {
                            defaultLineSize = defaultLineSize * 2;
                            line = (char *)realloc(line, defaultLineSize);
                        }
                        sz = read(fd, &ch, 1);
                        line[index] = ch;
                        index++;
                    }
                    line[index + 1] = '\0';
                    int tempFlag = 1;
                    if (index > 1)
                    {
                        for (int i = 0; i < strlen(argument); i++)
                        {
                            if (argument[i] != line[i] && !(argument[i] == ' ' && line[i] == '\0'))
                            {
                                tempFlag = 0;
                            }
                        }
                        if (tempFlag)
                        {
                            free(line);
                            return pid;
                        }
                    }
                }
            }
        }
        closedir(directory);
    }
    else
    {
        fprintf(stderr, "Folder couldn't found!\n");
        freeResources();
    }
    free(line);
    return -1;
}

int hashCode(char *key)
{
    int len = strlen(key);
    long int hash = 0;
    for (int i = 0; i < len; ++i)
        hash = 31 * hash + key[i];
    return (int)(hash % tableSize);
}

struct DataItem *search(char *key)
{
    int hashIndex = hashCode(key);
    while (hashArray[hashIndex] != NULL)
    {

        if (strcmp(hashArray[hashIndex]->key, key) == 0)
        {
            return hashArray[hashIndex];
        }
        else
        {
            hashIndex++;
            hashIndex %= tableSize;
        }
    }

    return NULL;
}
void insertToHash(char *key)
{

    if (totalItemNumber == tableSize)
    {
        reallocateHashTable();
    }
    struct DataItem *item = (struct DataItem *)malloc(sizeof(struct DataItem));
    item->data = NULL;

    strcpy(item->key, key);
    int hashIndex = hashCode(key);
    while (hashArray[hashIndex] != NULL && hashArray[hashIndex]->key != NULL && hashIndex < tableSize - 1)
    {
        hashIndex++;
        hashIndex %= tableSize;
    }
    totalItemNumber++;
    hashArray[hashIndex] = item;
}

struct DataItem *delete (struct DataItem *item) // freeleyerek silme işlemi yapılacak
{
    char key[100];
    strcpy(key, item->key);
    int hashIndex = hashCode(key);
    while (hashArray[hashIndex] != NULL)
    {

        if (strcmp(hashArray[hashIndex]->key, key) == 0)
        {
            struct DataItem *temp = hashArray[hashIndex];
            hashArray[hashIndex] = NULL;
            return temp;
        }
        else
        {
            hashIndex++;
            hashIndex %= tableSize;
        }
    }
    return NULL;
}
void reallocateHashTable()
{

    struct DataItem **temp = (struct DataItem **)malloc(sizeof(struct DataItem *) * tableSize * 2);
    for (int i = 0; i < tableSize; i++)
    {
        temp[i] = hashArray[i];
    }
    for (int i = 0; i < tableSize; i++)
    {
        hashArray[i] = NULL;
    }
    for (int i = 0; i < tableSize; i++)
    {
        if (temp[i] != NULL)
        {
            insertToHash(temp[i]->key);
        }
    }
    tableSize = tableSize * 2;
    free(temp);
}
int addEntry(char *cityName, entry *entry)
{
    struct DataItem *item = search(cityName);
    if (item == NULL)
    {
        return 0;
    }
    else
    {
        if (item->data == NULL)
        {
            item->data = insert_item(item->data, entry);
        }
        else
        {
            insert_item(item->data, entry);
        }

        return 0;
    }
}

int findEmptyPort(int start)
{
    int temp = start + PORT_START_NUMBER;

    while (isPortExit(temp))
    {
        temp++;
    }
    return temp;
}
void *servantHandler(void *arg)
{

    int new_socket_fd = dequeue(requestQueue);
    message temp;
    int read_control;
    if (new_socket_fd != -1)
    {
        read_control = read(new_socket_fd, &temp, sizeof(temp));
        if (read_control < 0)
        {
            fprintf(stderr, "Request read failed\n");
            freeResources();
        }
        if (temp.type == CLIENT_REQUEST)
        {
            request *req = (request *)temp.buffer;
            if (strcmp(req->city, "ALL") == 0)
            {
                int total = findAllCities(req);
                message response;
                response.type = SERVANT_RESPONSE;
                sprintf((char *)response.buffer, "%d", total);
                write(new_socket_fd, &response, sizeof(message));
                handledRequestNum++;
                close(new_socket_fd);
            }
            else
            {
                int total = findCities(req);
                message response;
                response.type = SERVANT_RESPONSE;
                sprintf((char *)response.buffer, "%d", total);
                write(new_socket_fd, &response, sizeof(message));
                handledRequestNum++;
                close(new_socket_fd);
            }
        }
        else if (temp.type == EXIT)
        {
            // response will be delivered to client
            printTime();
            printf("Servant: %d, termination message received, handled %d requests in total\n", processID, handledRequestNum);
            freeResources();
        }
        else
        {
            fprintf(stderr, "Invalid request type\n");
            freeResources();
        }
    }

    return NULL;
}
int findCities(request *req)
{
    struct DataItem *temp = search(req->city);
    int count = 0;
    if (temp != NULL)
    {
        countCities(temp->data, &count, req);
    }
    return count;
}
int findAllCities(request *req)
{
    int total = 0;
    for (int i = 0; i < tableSize; i++)
    {
        if (hashArray[i] != NULL)
        {
            countCities(hashArray[i]->data, &total, req);
        }
    }
    return total;
}
void countCities(struct entry_node *root, int *count, request *req)
{
    if (root != NULL)
    {
        if (compareDate(root->key.date, req->start_date) >= 0 && strcmp(root->key.real_estate, req->field_type) == 0 && compareDate(root->key.date, req->end_date) < 0)
        {
            *count = *count + 1;
        }

        countCities(root->left, count, req);
        countCities(root->right, count, req);
    }
}
void freeResources()
{

    if (cities != NULL)
    {
        for (int i = 0; i < city_count + 1; i++)
        {
            free(cities[i]);
        }
        free(cities);
        cities = NULL;
    }
    for (int i = 0; i < tableSize; i++)
    {
        if (hashArray[i] != NULL)
        {
            free_tree(hashArray[i]->data);
        }
        free(hashArray[i]);
        hashArray[i] = NULL;
    }

    if (requestQueue->array != NULL)
    {
        free(requestQueue->array);
        requestQueue->array = NULL;
    }
    if (requestQueue != NULL)
    {
        free(requestQueue);
        requestQueue = NULL;
    }
    if (fd_array != NULL)
    {
        free(fd_array);
        fd_array = NULL;
    }
    exit(EXIT_SUCCESS);
}

void free_tree(struct entry_node *node)
{
    // post-order like FatalError hinted at
    if (node != NULL)
    {
        free_tree(node->right);
        free_tree(node->left);
        free(node);
    }
}