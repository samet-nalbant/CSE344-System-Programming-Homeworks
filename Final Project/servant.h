#include "helper.h"
#define DEFAULT_TABLE_SIZE 1024
#define PORT_START_NUMBER 2000

struct entry // entry represent the inside of the file
{
    date date;
    int transaction_id;
    char real_estate[CHAR_ARRAY_SIZE];
    char street_name[CHAR_ARRAY_SIZE];
    int surface;
    int price;
};

typedef struct entry entry;

struct entry_node // binary tree for entries
{
    entry key;
    struct entry_node *left, *right;
};

struct DataItem // Hash table
{
    struct entry_node *data;
    char key[CHAR_ARRAY_SIZE];
};

struct DataItem *hashArray[DEFAULT_TABLE_SIZE];
int tableSize = DEFAULT_TABLE_SIZE;
int totalItemNumber = 0;
int findAllCities(request *req);
struct entry_node *createNode(entry *item);
void traverse(struct entry_node *root);
struct entry_node *insert_item(struct entry_node *node, entry *key);
int findCities(request *req);
void checkArguments(int argc, char **argv, char *directoryPath, char *ipAddress, int *port, int *start, int *end, char *combined);
void readFile(char *directoryPath, char *cityPath, struct entry_node **entry_root);
int parseLine(char *line, entry *item);
void sortCityNames(char **cities, int size);
int isPortExit(int portNumber);
int findProcessNumber(char *argument);
void combineArguments(int argc, char **argv, char *argument);
int findEmptyPort(int start);
int hashCode(char *key);
struct DataItem *search(char *key);
void insertToHash(char *key);
struct DataItem *delete (struct DataItem *item);
void reallocateHashTable();
int addEntry(char *cityName, entry *entry);
void *servantHandler(void *arg);
void countCities(struct entry_node *root, int *count, request *req);
void freeResources();
void free_tree(struct entry_node *node);