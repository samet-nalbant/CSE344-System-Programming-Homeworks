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
extern char *optarg;

void printMatrix(int **matrix, int matrixSize);

void checkArguments(int argc, char **argv, char *inputFile1, char *inputFile2, char *outputFile, int *n, int *m);

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void freeResources();
int **matrix1, **matrix2, **resultMatrix;
double **resultMatrixImg, **resultMatrixRe;
pthread_t *threadArray;
int fd = -1;
int fd2 = -1;
int *indexArray;
int matrixSize;
int m;
int c, total = 0;
int sig = 0;
void handler(int signalNum)
{
    if (signalNum == SIGINT)
    {
        sig = 1;
    }
}

pthread_mutex_t mtx;
void *multiplyCalculator(void *arg);

int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    if (sig)
    {
        freeResources();
        exit(EXIT_SUCCESS);
    }
    char inputFile1[100], inputFile2[100], outputFile[100], temp[100];
    time_t t1;
    int n, error;
    void *res;
    clock_t t;
    t = clock();
    checkArguments(argc, argv, inputFile1, inputFile2, outputFile, &n, &m);
    matrixSize = pow(2, n);
    matrix1 = (int **)malloc(sizeof(int *) * matrixSize);
    for (int i = 0; i < matrixSize; i++)
    {
        matrix1[i] = (int *)malloc(sizeof(int) * matrixSize);
    }
    matrix2 = (int **)malloc(sizeof(int *) * matrixSize);
    for (int i = 0; i < matrixSize; i++)
    {
        matrix2[i] = (int *)malloc(sizeof(int) * matrixSize);
    }
    resultMatrix = (int **)malloc(sizeof(int *) * matrixSize);
    for (int i = 0; i < matrixSize; i++)
    {
        resultMatrix[i] = (int *)malloc(sizeof(int) * matrixSize);
    }
    resultMatrixRe = (double **)malloc(sizeof(double *) * matrixSize);
    for (int i = 0; i < matrixSize; i++)
    {
        resultMatrixRe[i] = (double *)malloc(sizeof(double) * matrixSize);
    }
    resultMatrixImg = (double **)malloc(sizeof(double *) * matrixSize);
    for (int i = 0; i < matrixSize; i++)
    {
        resultMatrixImg[i] = (double *)malloc(sizeof(double) * matrixSize);
    }
    for (int i = 0; i < matrixSize; i++)
    {
        for (int j = 0; j < matrixSize; j++)
        {
            resultMatrixImg[i][j] = 0;
            resultMatrixRe[i][j] = 0;
        }
    }

    int sz = 1;
    char ch;
    int i = 0, j = 0;
    fd = open(inputFile1, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        fprintf(stderr, "Open File");
        freeResources();
        exit(EXIT_FAILURE);
    }
    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        if (sz > 0)
        {
            if (i == matrixSize)
            {
                break;
            }

            if (j == matrixSize - 1)
            {
                matrix1[i][j] = ch;
                i++;
                j = -1;
            }
            else
            {
                matrix1[i][j] = ch;
            }
            j++;
        }
    }
    close(fd);
    if (i != matrixSize && j != 0)
    {
        fprintf(stderr, "%s", "Input file is wrong!\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
    i = 0;
    j = 0;
    sz = 1;
    fd = open(inputFile2, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1)
    {
        fprintf(stderr, "%s", "Open File");
        freeResources();
        exit(EXIT_FAILURE);
    }
    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        if (sz > 0)
        {
            if (i == matrixSize)
            {
                break;
            }

            if (j == matrixSize - 1)
            {
                matrix2[i][j] = ch;
                i++;
                j = -1;
            }
            else
            {
                matrix2[i][j] = ch;
            }
            j++;
        }
    }
    close(fd);
    if (i != matrixSize && j != 0)
    {
        fprintf(stderr, "%s", " Input file is wrong !\n ");
        freeResources();
        exit(EXIT_FAILURE);
    }
    time(&t1);
    sprintf(temp, "%s", ctime(&t1));
    temp[strlen(temp) - 1] = '\0';
    printf("%s ------- Two matrices of size %dx%d have been read. The number of threads is %d\n", temp, matrixSize, matrixSize, m);

    threadArray = (pthread_t *)malloc(sizeof(pthread_t) * m);
    indexArray = (int *)malloc(sizeof(int) * m);

    pthread_mutexattr_t mtxAttr;
    int s;
    s = pthread_mutexattr_init(&mtxAttr);
    if (s != 0)
    {
        fprintf(stderr, "%s", " Pthread mutex attribute init\n ");
        freeResources();
        exit(EXIT_FAILURE);
    }
    s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_ERRORCHECK);
    if (s != 0)
    {
        fprintf(stderr, "%s", "Mutex set attr type error\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
    s = pthread_mutex_init(&mtx, &mtxAttr);
    if (s != 0)
    {
        fprintf(stderr, "%s", "Pthread mutex init\n");
        freeResources();
        exit(EXIT_FAILURE);
    }

    s = pthread_mutexattr_destroy(&mtxAttr);
    if (s != 0)
    {
        fprintf(stderr, "%s", "Mutex attr destroy error\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
    if (sig)
    {
        freeResources();
        exit(EXIT_SUCCESS);
    }
    for (int i = 0; i < m; i++)
    {
        indexArray[i] = i;

        if ((error = pthread_create(&threadArray[i], NULL, multiplyCalculator, &indexArray[i])))
        {
            fprintf(stderr, "%s", "Thread couldn't created\n");
            freeResources();
            exit(EXIT_FAILURE);
        }
        if (sig)
        {
            freeResources();
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < m; i++)
    {
        if (sig)
        {
            freeResources();
            exit(EXIT_SUCCESS);
        }
        if ((error = pthread_join(threadArray[i], &res)))
        {
            fprintf(stderr, "%s", "Thread couldn't joined\n");
            freeResources();
            exit(EXIT_FAILURE);
        }
    }
    // printMatrix(matrix1, matrixSize);
    // printMatrix(matrix2, matrixSize);

    if (sig)
    {
        freeResources();
        exit(EXIT_SUCCESS);
    }
    // printMatrix(resultMatrix, matrixSize);
    fd2 = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd2 == -1)
    {
        fprintf(stderr, "%s", "Open File Error");
        freeResources();
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < matrixSize; i++)
    {
        for (int j = 0; j < matrixSize; j++)
        {
            if (j == matrixSize - 1)
            {
                sprintf(temp, "%.3lf+i(%.3lf)", resultMatrixRe[i][j], resultMatrixImg[i][j]);
                write(fd2, temp, strlen(temp));
            }
            else
            {
                sprintf(temp, "%.3lf+i(%.3lf),", resultMatrixRe[i][j], resultMatrixImg[i][j]);
                write(fd2, temp, strlen(temp));
            }
        }
        if (i != matrixSize - 1)
            write(fd2, "\n", strlen("\n"));
    }
    t = clock() - t;
    time(&t1);
    sprintf(temp, "%s", ctime(&t1));
    temp[strlen(temp) - 1] = '\0';
    printf("%s ------- The process has written the output file. The total time spent is %f seconds.\n", temp, ((double)t) / CLOCKS_PER_SEC);
    // printMatrix(resultMatrix, matrixSize);
    for (int i = 0; i < matrixSize; i++)
    {
        free(resultMatrix[i]);
        free(resultMatrixImg[i]);
        free(resultMatrixRe[i]);
        free(matrix1[i]);
        free(matrix2[i]);
    }
    free(resultMatrix);
    free(resultMatrixImg);
    free(resultMatrixRe);
    free(matrix1);
    free(matrix2);
    free(threadArray);
    free(indexArray);
    return 0;
}

void checkArguments(int argc, char **argv, char *inputFile1, char *inputFile2, char *outputFile, int *n, int *m)
{
    int option;
    if (argc != 11)
    {
        fprintf(stderr, "%s", "Invalid Argument Number\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
    while ((option = getopt(argc, argv, "i:j:o:n:m:")) != -1)
    {
        switch (option)
        {
        case 'i':
            strcpy(inputFile1, optarg);
            break;
        case 'j':
            strcpy(inputFile2, optarg);
            break;
        case 'o':
            strcpy(outputFile, optarg);
            break;
        case 'n':
            *n = atoi(optarg);
            break;
        case 'm':
            *m = atoi(optarg);
            break;
        default:
            fprintf(stderr, "%s", "Invalid Arguments\n");
            freeResources();
            exit(EXIT_FAILURE);
        }
    }
    if (inputFile1 == NULL || inputFile2 == NULL || outputFile == NULL)
    {
        fprintf(stderr, "%s", "Invalid Arguments\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
    if (*n <= 2)
    {
        fprintf(stderr, "%s", "N must be greater than 2\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
    if (*m < 2)
    {
        fprintf(stderr, "%s", "M must be greater than 2\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
}

void printMatrix(int **matrix, int matrixSize)
{
    for (int i = 0; i < matrixSize; i++)
    {
        for (int j = 0; j < matrixSize; j++)
        {
            printf(" %d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void *multiplyCalculator(void *arg)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa, NULL);
    int index = *(int *)arg;
    clock_t t;
    time_t t1;
    char temp[100];
    t = clock();
    //*(int *)arg;
    int i, j, k, start, end;
    if (m > matrixSize)
    {
        start = index;
        end = (index + 1);
    }
    else
    {
        start = index * (int)floor(matrixSize / m);
        end = (index + 1) * (int)floor(matrixSize / m);
    }

    if (matrixSize % m != 0 && index == m - 1)
    {
        end = matrixSize;
    }
    if (index < matrixSize)
    {
        for (i = 0; i < matrixSize; i++)
        {
            for (j = start; j < end; j++)
            {
                resultMatrix[i][j] = 0;
                for (k = 0; k < matrixSize; k++)
                    resultMatrix[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    if (sig)
    {
        return NULL;
    }
    t = clock() - t;
    time(&t1);
    sprintf(temp, "%s", ctime(&t1));
    temp[strlen(temp) - 1] = '\0';
    printf("%s ------- Thread %d has reached the rendezvous point in %f seconds.\n", temp, index + 1, ((double)t) / CLOCKS_PER_SEC);
    pthread_mutex_lock(&mtx);

    ++total;
    while (total < m)
    {
        if (sig)
        {
            return NULL;
        }
        if (pthread_cond_wait(&cond, &mtx) != 0)
        {
            fprintf(stderr, "%s", "Condition wait error\n");
            freeResources();
            exit(EXIT_FAILURE);
        }
    }
    if (sig)
    {
        return NULL;
    }
    if (pthread_cond_broadcast(&cond) != 0)
    {
        fprintf(stderr, "%s", "Condition broadcast error\n");
        freeResources();
        exit(EXIT_FAILURE);
    }
    if (sig)
    {
        return NULL;
    }
    pthread_mutex_unlock(&mtx);
    time(&t1);
    sprintf(temp, "%s", ctime(&t1));
    temp[strlen(temp) - 1] = '\0';
    printf("%s ------- Thread %d is advancing to the second part\n", temp, index + 1);
    t = clock();
    if (sig)
    {
        return NULL;
    }
    if (index < matrixSize)
    {
        for (int i = 0; i < matrixSize; i++)
        {
            for (j = start; j < end; j++)
            {
                double ak = 0;
                double bk = 0;
                for (int ii = 0; ii < matrixSize; ii++)
                {

                    for (int jj = 0; jj < matrixSize; jj++)
                    {

                        double x = -2.0 * M_PI * i * ii / (double)matrixSize;
                        double y = -2.0 * M_PI * j * jj / (double)matrixSize;
                        ak += resultMatrix[ii][jj] * cos(x + y);
                        bk += resultMatrix[ii][jj] * 1.0 * sin(x + y);
                    }
                }
                resultMatrixRe[i][j] = ak;
                resultMatrixImg[i][j] = bk;
            }
        }
    }
    t = clock() - t;
    time(&t1);
    sprintf(temp, "%s", ctime(&t1));
    temp[strlen(temp) - 1] = '\0';
    printf("%s ------- Thread %d has has finished the second part in %f seconds.\n", temp, index + 1, ((double)t) / CLOCKS_PER_SEC);
    return NULL;
}
void freeResources()
{
    for (int i = 0; i < matrixSize; i++)
    {
        free(resultMatrix[i]);
        free(resultMatrixImg[i]);
        free(resultMatrixRe[i]);
        free(matrix1[i]);
        free(matrix2[i]);
    }
    free(resultMatrix);
    free(resultMatrixImg);
    free(resultMatrixRe);
    free(matrix1);
    free(matrix2);
    free(threadArray);
    free(indexArray);
    if (fd != -1)
        close(fd);
    if (fd2 != -1)
        close(fd2);
}