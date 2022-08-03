

#include "helper.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// return 0 on error
int parseDate(char *str, date *date)
{
    char *token;

    int count = 0;

    while ((token = strtok_r(str, "-", &str)))
    {
        switch (count)
        {
        case 0:
            date->day = atoi(token);
            break;
        case 1:
            date->month = atoi(token);
            break;
        case 2:
            date->year = atoi(token);
            break;
        default:
            break;
        }
        count++;
    }

    free(token);
    if (count <= 2 || count > 3)
    {
        date = NULL;
        return 0;
    }
    return 1;
}

int getArgNum(char *line)
{
    int i = 0;
    int count = 0;
    while (line[i] != '\0')
    {
        if (line[i] == ' ')
        {
            count++;
        }
        i++;
    }
    return count;
}
void printDate(date date)
{
    printf("%d-%d-%d ", date.day, date.month, date.year);
}
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int *)malloc(
        queue->capacity * sizeof(int));
    return queue;
}
int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}
void enqueue(struct Queue *queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

int dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
        return -1;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

int front(struct Queue *queue)
{
    if (isEmpty(queue))
        return -1;
    return queue->array[queue->front];
}

int rear(struct Queue *queue)
{
    if (isEmpty(queue))
        return -1;
    return queue->array[queue->rear];
}

struct node *newNode(char *item)
{
    struct node *temp = (struct node *)malloc(sizeof(struct node));
    strcpy(temp->key, item);
    temp->left = temp->right = NULL;
    return temp;
}

void inorder(struct node *root)
{
    if (root != NULL)
    {
        inorder(root->left);
        printf("%s\n", root->key);
        inorder(root->right);
    }
}

struct node *insert(struct node *node, char *key)
{

    if (node == NULL)
        return newNode(key);
    if (strcmp(key, node->key) < 0)
    {
        node->left = insert(node->left, key);
    }
    else if (strcmp(key, node->key) > 0)
        node->right = insert(node->right, key);
    return node;
}
// returns 0 if they are equal, -1 on d2 greater, 1 on d1 greater
int compareDate(date dt1, date dt2)
{
    if (dt1.day == dt2.day && dt1.month == dt2.month && dt1.year == dt2.year)
    {
        return 0;
    }
    if (dt1.year < dt2.year)
    {
        return -1;
    }
    else if (dt1.year > dt2.year)
    {
        return 1;
    }
    else if (dt1.month < dt2.month)
    {
        return -1;
    }
    else if (dt1.month > dt2.month)
    {
        return 1;
    }
    else if (dt1.day < dt2.day)
    {
        return -1;
    }
    return 1;
}

int getLength(char *str)
{
    int i = 0;
    if (str == NULL)
    {
        return 0;
    }

    while (str[i] != '\0')
    {
        i++;
    }
    return i;
}
void printTime()
{
    time_t t;
    char output[BUFFER_SIZE];
    time(&t);
    sprintf(output, "%s", ctime(&t));
    output[strlen(output) - 1] = '\0';
    printf("%s ---- ", output);
}
void printRequest(request req)
{
    printf("%s %s ", req.transcationCount, req.field_type);
    printDate(req.start_date);
    printDate(req.start_date);
    if (strcmp(req.city, "ALL") != 0)
        printf("%s", req.city);
}