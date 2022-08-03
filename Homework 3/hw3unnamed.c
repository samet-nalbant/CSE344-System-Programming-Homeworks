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
#include <getopt.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <math.h>

extern char *optarg;
void checkArguments(int argc, char **argv, char *filePath);
void *create_shared_memory(size_t size)
{
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;
    return mmap(NULL, size, protection, visibility, -1, 0);
}

struct chef
{
    pid_t pid;
    int id;
};

typedef struct chef chef;
int numOfDesserts = 0;
void handler(int sig_num)
{
    exit(numOfDesserts);
}
int main(int argc, char *argv[])
{
    char filePath[100];
    struct sigaction sa;
    int fd;
    int pid = 1;
    int pusherA, pusherB, pusherC, pusherD, chef0, chef1, chef2, chef3, chef4, chef5;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    chef chefArray[6];
    checkArguments(argc, argv, filePath);
    char *shmem = (char *)create_shared_memory(128);
    chef *shmemChefs = (chef *)create_shared_memory(sizeof(chef) * 6);
    char array[4];
    array[3] = '\0';
    fd = open(filePath, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        perror("Open File");
    }
    int sz = 1;

    sem_t *semMilk = mmap(NULL, sizeof(*semMilk), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semMilk == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semMilk, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semFlour = mmap(NULL, sizeof(*semFlour), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semFlour == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semFlour, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semWalnuts = mmap(NULL, sizeof(*semWalnuts), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semWalnuts == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semWalnuts, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semSugar = mmap(NULL, sizeof(*semSugar), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semSugar == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semSugar, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semMilkFlour = mmap(NULL, sizeof(*semMilkFlour), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semMilkFlour == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semMilkFlour, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semSugarMilk = mmap(NULL, sizeof(*semSugarMilk), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semSugarMilk == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semSugarMilk, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semMilkWalnuts = mmap(NULL, sizeof(*semMilkWalnuts), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semMilkWalnuts == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semMilkWalnuts, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semWalnutsSugar = mmap(NULL, sizeof(*semWalnutsSugar), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semWalnutsSugar == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semWalnutsSugar, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semFlourSugar = mmap(NULL, sizeof(*semFlourSugar), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semFlourSugar == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semFlourSugar, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semFlourWalnuts = mmap(NULL, sizeof(*semFlourWalnuts), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semFlourWalnuts == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(semFlourWalnuts, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *m = mmap(NULL, sizeof(*m), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (m == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(m, 1, 1) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *agent = mmap(NULL, sizeof(*agent), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (agent == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(agent, 1, 1) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 6; i++)
    {
        if (pid != 0)
        {
            pid = fork();
            if (pid != 0)
            {
                chefArray[i].pid = pid;
                chefArray[i].id = i;
            }
        }
    }
    if (pid != 0)
    {
        memcpy(shmemChefs, chefArray, sizeof(chefArray));
        printf("chef %d (pid %d) is waiting for walnuts and sugar - charachter array: %s\n", 0, shmemChefs[0].pid, shmem);
        printf("chef %d (pid %d) is waiting for flour and walnuts - charachter array: %s\n", 1, shmemChefs[1].pid, shmem);
        printf("chef %d (pid %d) is waiting for sugar and flour - charachter array: %s\n", 2, shmemChefs[2].pid, shmem);
        printf("chef %d (pid %d) is waiting for milk and flour - charachter array: %s\n", 3, shmemChefs[3].pid, shmem);
        printf("chef %d (pid %d) is waiting for milk and walnuts - charachter array: %s\n", 4, shmemChefs[4].pid, shmem);
        printf("chef %d (pid %d) is waiting for sugar and milk - charachter array: %s\n", 5, shmemChefs[5].pid, shmem);
    }
    for (;;)
    {
        if (pid != 0) // parent
        {
            while (sz > 0)
            {
                sem_wait(agent);
                sz = read(fd, &array, 3);
                if (sz > 0)
                {
                    if (array[2] != '\n')
                    {
                        perror("Input file is wrong!\n");
                        exit(EXIT_FAILURE);
                    }
                    strncpy(shmem, array, 2);
                    // printf("Shared Memory: %s\n\n", shmem);
                    //   printf("Readed Value : %s\n", shmem);
                    // printf("Shmem 0: %c, Shmem 1: %c\n", shmem[0], shmem[1]);
                    switch (shmem[0])
                    {
                    case 'S':
                        sem_post(semSugar);
                        break;
                    case 'M':
                        sem_post(semMilk);
                        break;
                    case 'W':
                        sem_post(semWalnuts);
                        break;
                    case 'F':
                        sem_post(semFlour);
                        break;
                    default:
                        break;
                    }
                    switch (shmem[1])
                    {
                    case 'S':
                        sem_post(semSugar);
                        break;
                    case 'M':
                        sem_post(semMilk);
                        break;
                    case 'W':
                        sem_post(semWalnuts);
                        break;
                    case 'F':
                        sem_post(semFlour);
                        break;
                    default:
                        break;
                    }
                    int isMilk = 0, isSugar = 0, isWalnuts = 0, isFlour = 0;
                    sem_getvalue(semMilk, &pusherA);
                    sem_getvalue(semSugar, &pusherB);
                    sem_getvalue(semFlour, &pusherC);
                    sem_getvalue(semWalnuts, &pusherD);
                    if (pusherA)
                    {
                        if (sem_trywait(semMilk) != -1)
                        {
                            sem_wait(m);
                            if (isFlour)
                            {
                                isFlour = 0;
                                sem_post(semMilkFlour);
                            }
                            else if (isSugar)
                            {
                                isSugar = 0;
                                sem_post(semSugarMilk);
                            }
                            else if (isWalnuts)
                            {
                                isWalnuts = 0;
                                sem_post(semMilkWalnuts);
                            }
                            else
                            {
                                isMilk = 1;
                            }
                            sem_post(m);
                        }
                    }
                    if (pusherB)
                    {
                        if (sem_trywait(semSugar) != -1)
                        {
                            sem_wait(m);

                            if (isFlour)
                            {
                                isFlour = 0;
                                sem_post(semFlourSugar);
                            }
                            else if (isMilk)
                            {
                                isMilk = 0;
                                sem_post(semSugarMilk);
                            }
                            else if (isWalnuts)
                            {
                                isWalnuts = 0;
                                sem_post(semWalnutsSugar);
                            }
                            else
                            {
                                isSugar = 1;
                            }
                            sem_post(m);
                        }
                    }
                    if (pusherC)
                    {
                        if (sem_trywait(semFlour) != -1)
                        {
                            sem_wait(m);
                            if (isMilk)
                            {
                                isMilk = 0;
                                sem_post(semMilkFlour);
                            }
                            else if (isSugar)
                            {
                                isSugar = 0;
                                sem_post(semFlourSugar);
                            }
                            else if (isWalnuts)
                            {
                                isWalnuts = 0;
                                sem_post(semFlourWalnuts);
                            }
                            else
                            {
                                isFlour = 1;
                            }
                            sem_post(m);
                        }
                    }
                    if (pusherD)
                    {
                        if (sem_trywait(semWalnuts) != -1)
                        {
                            sem_wait(m);
                            if (isFlour)
                            {
                                isFlour = 0;
                                sem_post(semFlourWalnuts);
                            }
                            else if (isSugar)
                            {
                                isSugar = 0;
                                sem_post(semWalnutsSugar);
                            }
                            else if (isWalnuts)
                            {
                                isMilk = 0;
                                sem_post(semMilkWalnuts);
                            }
                            else
                            {
                                isWalnuts = 1;
                            }
                            sem_post(m);
                        }
                    }
                    int flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0, flag5 = 0, flag6 = 0;
                    sem_getvalue(semSugarMilk, &flag1);
                    sem_getvalue(semFlourSugar, &flag2);
                    sem_getvalue(semWalnutsSugar, &flag3);
                    sem_getvalue(semMilkFlour, &flag4);
                    sem_getvalue(semMilkWalnuts, &flag5);
                    sem_getvalue(semFlourWalnuts, &flag6);
                    if (flag1)
                    {
                        printf("the wholesaler (pid %d) delivers %s and %s - charachter array: %s\n", getpid(), "Sugar", "Milk", shmem);
                        printf("the wholesaler (pid %d) is waiting for the dessert - charachter array: %s\n", getpid(), shmem);
                    }
                    else if (flag2)
                    {
                        printf("the wholesaler (pid %d) delivers %s and %s - charachter array: %s\n", getpid(), "Flour", "Sugar", shmem);
                        printf("the wholesaler (pid %d) is waiting for the dessert - charachter array: %s\n", getpid(), shmem);
                    }
                    else if (flag3)
                    {
                        printf("the wholesaler (pid %d) delivers %s and %s - charachter array: %s\n", getpid(), "Walnuts", "Sugar", shmem);
                        printf("the wholesaler (pid %d) is waiting for the dessert - charachter array: %s\n", getpid(), shmem);
                    }
                    else if (flag4)
                    {
                        printf("the wholesaler (pid %d) delivers %s and %s - charachter array: %s\n", getpid(), "Milk", "Flour", shmem);
                        printf("the wholesaler (pid %d) is waiting for the dessert - charachter array: %s\n", getpid(), shmem);
                    }
                    else if (flag5)
                    {
                        printf("the wholesaler (pid %d) delivers %s and %s - charachter array: %s\n", getpid(), "Milk", "Walnuts", shmem);
                        printf("the wholesaler (pid %d) is waiting for the dessert - charachter array: %s\n", getpid(), shmem);
                    }
                    else if (flag6)
                    {
                        printf("the wholesaler (pid %d) delivers %s and %s - charachter array: %s\n", getpid(), "Flour", "Walnuts", shmem);
                        printf("the wholesaler (pid %d) is waiting for the dessert - charachter array: %s\n", getpid(), shmem);
                    }
                }
            }
            int totalDeserts = 0;
            int status;
            for (int i = 0; i < 6; i++)
            {
                kill(chefArray[i].pid, SIGINT);
                waitpid(chefArray[i].pid, &status, 0);
                totalDeserts += WEXITSTATUS(status);
                printf("chef %d (pid %d) is exiting - charachter array: %s\n", i, chefArray[i].pid, shmem);
            }

            printf("the wholesaler (pid %d) is done (total desserts: %d) - charachter array: %s\n", getpid(), totalDeserts, shmem);
            exit(0);
        }
        else
        {
            sem_getvalue(semWalnutsSugar, &chef0);
            sem_getvalue(semFlourWalnuts, &chef1);
            sem_getvalue(semFlourSugar, &chef2);
            sem_getvalue(semMilkFlour, &chef3);
            sem_getvalue(semMilkWalnuts, &chef4);
            sem_getvalue(semSugarMilk, &chef5);

            // printf("Walnuts Sugar: %d, Flour Walnuts: %d, Flour Sugar: %d, Sugar Milk: %d, Milk Walnuts :%d, Sugar Milk: %d\n", chef0, chef1, chef2, chef3, chef4, chef5);
            if (chef0 && getpid() == shmemChefs[0].pid)
            {
                chef0 = 0;
                sem_wait(semWalnutsSugar);
                printf("chef %d (pid %d) has taken the Walnuts - charachter array: %s\n", shmemChefs[0].id, getpid(), shmem);
                printf("chef %d (pid %d) has taken the Sugar - charachter array: %s\n", shmemChefs[0].id, getpid(), shmem);
                printf("chef %d (pid %d) is preparing the dessert - charachter array: %s\n", shmemChefs[0].id, getpid(), shmem);
                printf("chef %d (pid %d) has delivered the dessert - charachter array: %s\n", shmemChefs[0].id, getpid(), shmem);
                numOfDesserts++;
                sem_post(agent);
            }
            if (chef1 && getpid() == shmemChefs[1].pid)
            {
                chef1 = 0;
                sem_wait(semFlourWalnuts);
                printf("chef %d (pid %d) has taken the Walnuts - charachter array: %s\n", shmemChefs[1].id, getpid(), shmem);
                printf("chef %d (pid %d) has taken the Flour - charachter array: %s\n", shmemChefs[1].id, getpid(), shmem);
                printf("chef %d (pid %d) is preparing the dessert - charachter array: %s\n", shmemChefs[1].id, getpid(), shmem);
                printf("chef %d (pid %d) has delivered the dessert - charachter array: %s\n", shmemChefs[1].id, getpid(), shmem);
                numOfDesserts++;
                sem_post(agent);
            }
            if (chef2 && getpid() == shmemChefs[2].pid)
            {
                chef2 = 0;
                sem_wait(semFlourSugar);
                printf("chef %d (pid %d) has taken the Sugar - charachter array: %s\n", shmemChefs[2].id, getpid(), shmem);
                printf("chef %d (pid %d) has taken the Flour - charachter array: %s\n", shmemChefs[2].id, getpid(), shmem);
                printf("chef %d (pid %d) is preparing the dessert - charachter array: %s\n", shmemChefs[2].id, getpid(), shmem);
                printf("chef %d (pid %d) has delivered the dessert - charachter array: %s\n", shmemChefs[2].id, getpid(), shmem);
                numOfDesserts++;
                sem_post(agent);
            }
            if (chef3 && getpid() == shmemChefs[3].pid)
            {
                chef3 = 0;
                sem_wait(semMilkFlour);
                printf("chef %d (pid %d) has taken the Milk - charachter array: %s\n", shmemChefs[3].id, getpid(), shmem);
                printf("chef %d (pid %d) has taken the Flour - charachter array: %s\n", shmemChefs[3].id, getpid(), shmem);
                printf("chef %d (pid %d) is preparing the dessert - charachter array: %s\n", shmemChefs[3].id, getpid(), shmem);
                printf("chef %d (pid %d) has delivered the dessert - charachter array: %s\n", shmemChefs[3].id, getpid(), shmem);
                numOfDesserts++;
                sem_post(agent);
            }
            if (chef4 && getpid() == shmemChefs[4].pid)
            {
                chef4 = 0;
                sem_wait(semMilkWalnuts);
                printf("chef %d (pid %d) has taken the Milk - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                printf("chef %d (pid %d) has taken the Walnuts - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                printf("chef %d (pid %d) is preparing the dessert - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                printf("chef %d (pid %d) has delivered the dessert - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                numOfDesserts++;
                sem_post(agent);
            }
            if (chef5 && getpid() == shmemChefs[5].pid)
            {
                chef5 = 0;
                sem_wait(semSugarMilk);
                printf("chef %d (pid %d) has taken the Milk - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                printf("chef %d (pid %d) has taken the Sugar - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                printf("chef %d (pid %d) is preparing the dessert - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                printf("chef %d (pid %d) has delivered the dessert - charachter array: %s\n", shmemChefs[4].id, getpid(), shmem);
                numOfDesserts++;
                sem_post(agent);
            }
        }
    }
    return 0;
}
void checkArguments(int argc, char **argv, char *filePath)
{
    int option;
    if (argc != 3)
    {
        perror("Invalid Argument Number!\n");
        exit(EXIT_FAILURE);
    }
    while ((option = getopt(argc, argv, "i:")) != -1)
    {
        switch (option)
        {
        case 'i':
            strcpy(filePath, optarg);
            break;
        default:
            perror("Invalid Arguments");
            exit(EXIT_FAILURE);
        }
    }
    if (filePath == NULL)
    {
        perror("Invalid Arguments");
        exit(EXIT_FAILURE);
    }
}