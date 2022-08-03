#define ARRAY_SIZE 30
#define BUFFER_SIZE 1024
#define CHAR_ARRAY_SIZE 100
#define MESSAGE_BUFFER_SIZE 103000
#pragma once
#define IN_USE 1
#define FREE 0
#include <stdint.h>
enum messageType
{
    CLIENT_REQUEST,
    RESPONSE,
    EXIT,
    SERVANT_DECLARATION,
    SERVANT_RESPONSE,
    NO_CITY_RESPONSE,
};
struct date
{
    int day;
    int month;
    int year;
};
typedef struct date date;
struct request
{
    char transcationCount[ARRAY_SIZE];
    char field_type[ARRAY_SIZE];
    char city[ARRAY_SIZE];
    date start_date;
    date end_date;
};

struct message
{
    enum messageType type;
    uint8_t buffer[MESSAGE_BUFFER_SIZE];
};

struct servantDeclaration
{
    int port;
    int processID;
    int total_city_number;
    char cities[BUFFER_SIZE][CHAR_ARRAY_SIZE];
};

typedef struct servantDeclaration servantDeclaration;
typedef struct request request;
typedef struct message message;
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int *array;
};

int parseDate(char *str, date *date);
int getArgNum(char *line);
void printDate(date date);
int getLength(char *str);
int front(struct Queue *queue);
int rear(struct Queue *queue);
int dequeue(struct Queue *queue);
void enqueue(struct Queue *queue, int item);
int isEmpty(struct Queue *queue);
struct Queue *createQueue(unsigned capacity);
int compareDate(date dt1, date dt2);
struct node
{
    char key[ARRAY_SIZE];
    struct node *left, *right;
};
struct node *newNode(char *item);
void inorder(struct node *root);
struct node *insert(struct node *node, char *key);
void printTime();
void printRequest(request req);