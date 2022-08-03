#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
typedef struct Coordinate
{
    int x, y, z;

} Coordinate;
int signal_flag = 0;

void handler(int signal_number)
{
    if (signal_number == SIGINT)
    {
        signal_flag = 1;
    }
}
struct sigaction sa;

void calculateCovarianceMatrix(Coordinate array[10], int count, char *filePath, char *id);
void writeToFile(double matrix[3][3], char *filePath, char *id);
void printCoordinates(Coordinate array[10], char *id);
int main(int argc, char *argv[], char *env[])
{
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGINT, NULL, &sa) == -1)
    {
    }
    char id[99];
    Coordinate array[10];
    strcpy(id, env[10]);
    for (int i = 0; i < 10; i++)
    {
        array[i].x = env[i][0];
        array[i].y = env[i][1];
        array[i].z = env[i][2];
    }
    printCoordinates(array, id);
    calculateCovarianceMatrix(array, 10, argv[0], id);
    exit(0);
}

void calculateCovarianceMatrix(Coordinate array[10], int count, char *filePath, char *id)
{
    double x_ = 0.0, y_ = 0.0, z_ = 0.0;
    double var_x = 0.0, var_y = 0.0, var_z = 0.0;
    double cov_xy = 0.0, cov_xz = 0.0, cov_yz = 0.0;
    double matrix[3][3];
    for (int i = 0; i < count; i++)
    {
        x_ += array[i].x / 10.0;
        y_ += array[i].y / 10.0;
        z_ += array[i].z / 10.0;
    }

    for (int i = 0; i < count; i++)
    {
        var_x += (x_ - array[i].x) * (x_ - array[i].x) / 10.0;
        var_y += (y_ - array[i].y) * (y_ - array[i].y) / 10.0;
        var_z += (z_ - array[i].z) * (z_ - array[i].z) / 10.0;
    }

    for (int i = 0; i < count; i++)
    {
        cov_xy += (array[i].x - x_) * (array[i].y - y_) / 10.0;
        cov_xz += (array[i].x - x_) * (array[i].z - z_) / 10.0;
        cov_yz += (array[i].y - y_) * (array[i].z - z_) / 10.0;
    }

    matrix[0][0] = var_x;
    matrix[0][1] = cov_xy;
    matrix[0][2] = cov_xz;
    matrix[1][0] = cov_xy;
    matrix[1][1] = var_y;
    matrix[1][2] = cov_yz;
    matrix[2][0] = cov_xz;
    matrix[2][1] = cov_yz;
    matrix[2][2] = var_z;
    writeToFile(matrix, filePath, id);
}

void writeToFile(double matrix[3][3], char *filePath, char *id)
{
    struct flock lock;
    int fd;
    char temp[99];
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    fd = open(filePath, O_WRONLY | O_CREAT | O_APPEND, mode);
    if (fd == -1)
        perror("open");
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);
    if (write(fd, id, strlen(id)) < 0)
    {
        perror("write");
    }
    if (write(fd, "\n", 1) < 0)
    {
        perror("write");
    }
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            sprintf(temp, "%.3f", matrix[i][j]);
            if (write(fd, temp, strlen(temp)) < 0)
            {
                perror("write");
            }
            if (j != 2)
            {
                if (write(fd, ",", 1) < 0)
                {
                    perror("write");
                }
            }
        }
        if (write(fd, "\n", 1) < 0)
        {
            perror("write");
        }
    }
    if (signal_flag == 1)
    {
        printf("SIGINT CATCHED!\n");
        exit(EXIT_SUCCESS);
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    if (close(fd) == -1)
        perror("close");
}
void printCoordinates(Coordinate array[10], char *id)
{
    printf("Created %s with ", id);
    for (int i = 0; i < 9; i++)
    {
        printf("(%d,%d,%d),", array[i].x, array[i].y, array[i].z);
    }
    printf("(%d,%d,%d)", array[9].x, array[9].y, array[9].z);
    printf("\n");
}