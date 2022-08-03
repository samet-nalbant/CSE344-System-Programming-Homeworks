#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct request
{
    pid_t pid;
    char matrix[1024];
    int p[2];
};

typedef struct request request;
