
#pragma once
#define CHAR_ARRAY_SIZE 100
#include "helper.h"

struct portItem // Hash table
{
    int port;
    int processID;
    char cities[BUFFER_SIZE][CHAR_ARRAY_SIZE];
    int city_count;
};
typedef struct portItem portItem;

void handleSigint();
void checkArguments(int argc, char **argv, int *port, int *numThreads);
void *serverHandler(void *arg);
void addToPortList(servantDeclaration *item);
portItem findPortNumber(char *city);
void free_serverResources();
