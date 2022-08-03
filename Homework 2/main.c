#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
void read_file(char *argv[]);
void read_output(char *filePath);
void clear_output(char *filePath);

int signal_flag = 0;
void handler(int signal_number)
{
    if (signal_number == SIGINT)
    {
        signal_flag = 1;
    }
}
int main(int argc, char *argv[])
{
    if (argc != 5 || (strcmp("-i", argv[1]) != 0) || (strcmp("-o", argv[3]) != 0))
    {
        perror("Invalid input\n");
        return 0;
    }
    clear_output(argv[4]);
    signal(SIGINT, handler);

    read_file(argv);
    while (wait(NULL) > 0)
    {
    }
    read_output(argv[4]);
    return 0;
}
void read_file(char *argv[])
{
    pid_t child_pid;
    pid_t *childIDlist = (pid_t *)malloc(sizeof(pid_t) * 10);
    int size = 10;
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGINT, NULL, &sa) == -1)
    {
    }
    char **env = (char **)malloc(sizeof(char *) * 11);
    for (int i = 0; i < 10; i++)
    {
        env[i] = (char *)malloc(sizeof(char) * 3);
    }
    int sz = 1;
    int flag = 0;
    int count = 0;
    char ch;
    int child_count = -1;
    char id[99];
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    printf("Process P reading %s\n", argv[2]);
    int fd = open(argv[2], O_RDONLY, mode);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        switch (flag)
        {
        case 0:
            env[count][0] = ch;
            break;
        case 1:
            env[count][1] = ch;
            break;
        case 2:
            env[count][2] = ch;
            flag = -1;
            count++;
            break;
        default:
            break;
        }
        if (count == 10)
        {
            child_count++;
            if (child_count == size)
            {
                childIDlist = realloc(childIDlist, size * 2 * sizeof(pid_t));
                size *= 2;
            }
            switch (child_pid = fork())
            {
            case -1:
                write(STDERR_FILENO, "Error", strlen("Error"));
                exit(EXIT_FAILURE);
                break;
            case 0:
                childIDlist[child_count - 1] = child_pid;
                sprintf(id, "R_%d", child_count);
                env[10] = malloc(sizeof(strlen(id)));
                strcpy(env[10], id);
                env[11] = NULL;
                execve("child", &argv[4], env);
                break;
            default:
                break;
            }
            count = 0;
        }
        flag++;

        if (signal_flag == 1)
        {
            printf("SIGINT CATCHED!\nChilds Are Killing\n");
            for (int i = 0; i < child_count; i++)
            {
                if (kill(childIDlist[child_count], SIGINT) == -1)
                {
                    exit(EXIT_FAILURE);
                }
            }
            exit(EXIT_SUCCESS);
        }
    }

    if (close(fd) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    free(childIDlist);
    for (int i = 0; i < 10; i++)
    {
        free(env[i]);
    }
    free(env);
}
void read_output(char *filePath)
{
    printf("Reached EOF, collecting outputs from %s\n", filePath);
    char ch;
    int sz = 1;
    int index1 = -1, index2 = -1;
    double min = 9999;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    int fd = open(filePath, O_RDONLY, mode);

    char value[99];
    int count = 0, count2 = 0; // count2 frobenous list index
    int column = 0, row = 0;
    int count3 = 0;
    int line = 0;
    double total = 0;
    int flag = 0;
    double *frobenious_list;
    double **matrix_list = malloc(sizeof(double *));
    char **id_list = malloc(sizeof(char *));
    matrix_list[0] = malloc(sizeof(double) * 3);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        if (ch == ' ')
        {
            break;
        }
        else if (ch == 10)
        {
            if (line % 4 == 0)
            {
                flag = 0;
            }
            line++;
            if (flag == 0)
            {
                value[count] = '\0';
                id_list[count3] = (char *)malloc(strlen(value) * sizeof(char));
                strcpy(id_list[count3], value);
                count3++;
                id_list = realloc(id_list, sizeof(char *) * count3 * 2);
                count = 0;
                flag = 1;
            }
            else
            {
                value[count] = '\0';
                matrix_list[column][row] = atof(value);
                row++;
                count = 0;
            }
        }
        else if (ch == ',')
        {
            value[count] = '\0';
            matrix_list[column][row] = atof(value);
            row++;
            count = 0;
        }
        else
        {
            value[count] = ch;
            count++;
        }
        if (row == 3)
        {
            column++;
            matrix_list = realloc(matrix_list, sizeof(double *) * column * 2);
            matrix_list[column] = malloc(sizeof(double) * 3);
            row = 0;
        }
    }
    if (close(fd) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }

    frobenious_list = (double *)malloc(sizeof(double) * column / 3 + 1);
    count2 = 0;

    for (int i = 0; i < column - 1; i++)
    {
        total += matrix_list[i][0] * matrix_list[i][0];
        total += matrix_list[i][1] * matrix_list[i][1];
        total += matrix_list[i][2] * matrix_list[i][2];
        if (i % 3 == 2)
        {
            frobenious_list[count2] = sqrt(total);
            count2++;
            total = 0;
        }
    }

    for (int i = 0; i < column / 3; i++)
    {
        for (int j = i + 1; j < column / 3; j++)
        {
            if (fabs(frobenious_list[i] - frobenious_list[j]) < min)
            {
                min = fabs(frobenious_list[i] - frobenious_list[j]);
                index1 = i;
                index2 = j;
            }
        }
    }

    printf("The closest 2 matrices are %s and %s, and their distance is %.3f\n", id_list[index1], id_list[index2], min);
    free(frobenious_list);
    for (int i = 0; i < column + 1; i++)
    {
        free(matrix_list[i]);
    }
    free(matrix_list);
    for (int i = 0; i < count3; i++)
    {
        free(id_list[i]);
    }
    free(id_list);
}
void clear_output(char *filePath)
{
    int fd;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    fd = open(filePath, O_CREAT | O_WRONLY | O_TRUNC, mode);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (close(fd) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}