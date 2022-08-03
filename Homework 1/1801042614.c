#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SINGLE_OPERATION 0
#define MULTIPLE_OPERATION 1

#define CASE_SENSITIVE 0
#define CASE_INSENSITIVE 1

#define NO_LINE_SYMBOL 0
#define START_OF_LINE 1
#define END_OF_LINE 2

typedef struct Buffer
{
    int word_num;
    char **content;
} Buffer;

int check_chars(char char1, char char2, int mode); // check two charachters they are equal or not according to mode which represents the CASE_SENSITIVE or CASE_INSENSITIVE

void read_file(char *fpath, char *str1, char *str2, int mode); // mode case sensitive or insensitive

void write_file(char *file_path, Buffer text); // Writes buffer to the given file which belongs to given file path

void parse_arguments(int argc, char *argv[]); // Parses given arguments according to needs

int find_operation(char *arg); // finds single or multiple operation

int find_case(char *arg); // finds case sensitive or not

int check_line(char *arg); // check there is end of line or start of line

char *remove_symbols(char *string); // removes unnecessary symbols

int check_alternative(char *string, char *expression, int mode); // checks the expression and string

char *check_words(char *word, char *expression, char *string, int mode);

int check_input(char *string);

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);
}

int check_chars(char char1, char char2, int mode)
{
    if (mode == CASE_SENSITIVE)
    {
        return char1 == char2;
    }
    else
    {
        return (char1 == char2 || (char1 - 32 == char2 || char1 + 32 == char2));
    }
}

void parse_arguments(int argc, char *argv[])
{
    char *token, *rest = NULL;
    int flag = 1;
    int arg_num = 0;
    int type;
    if (argc != 3)
    {
        perror("Invalid input\n");
        return;
    }
    int operation = find_operation(argv[1]);
    char *arg_array[3];

    token = strtok_r(argv[1], ";", &rest);
    while (token != NULL)
    {
        if (token[0] != '/')
        {
            perror("Wrong input format\n");
            return;
        }
        arg_num = 0;
        type = find_case(token);
        char *token2 = strtok(token, "/");

        while (token2 != NULL)
        {
            arg_array[arg_num] = (char *)malloc(strlen(token2));
            strcpy(arg_array[arg_num], token2);
            arg_num++;
            token2 = strtok(NULL, "/");
        }
        switch (operation)
        {
        case SINGLE_OPERATION:
            if (check_input(arg_array[0]) < 0)
            {
                return;
            }
            read_file(argv[2], arg_array[0], arg_array[1], type);
            break;
        case MULTIPLE_OPERATION:
            if (flag)
            {
                if (check_input(arg_array[0]) < 0)
                {
                    return;
                }
                read_file(argv[2], arg_array[0], arg_array[1], type);
                flag = 0;
            }
            else
            {
                if (check_input(arg_array[0]) < 0)
                {
                    return;
                }
                read_file(argv[2], arg_array[0], arg_array[1], type);
                flag = 0;
            }
            break;
        default:
            perror("Invalid input\n");
            break;
        }
        token = strtok_r(NULL, ";", &rest);
    }
    // free(rest);
    free(token);
}
void read_file(char *fpath, char *str1, char *str2, int mode)
{
    char *word;
    int sz = 1;
    int word_len = 0;
    Buffer word_list;
    word_list.word_num = 0;
    char ch;
    struct flock lock;
    int start_line_flag = 0;
    int first_word = 1;
    word_list.content = malloc(sizeof(char *));
    word = (char *)malloc(sizeof(char));
    int fd = open(fpath, O_RDONLY);

    int line_case = check_line(str1); // line case for end line or start line
    str1 = remove_symbols(str1);
    if (fd == -1)
    {
        perror("open");
    }
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);
    while (sz > 0)
    {
        sz = read(fd, &ch, 1);
        if (sz > 0)
        {
            if (ch == 10) // new line
            {
                start_line_flag = 1;
                if (word_len > 0)
                {
                    if (line_case == END_OF_LINE)
                    {
                        word = check_words(word, str1, str2, mode);
                        word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                        strcpy(word_list.content[word_list.word_num], word);
                        word_list.word_num++;
                        word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                        word_len = 0;
                        free(word);
                        word = (char *)malloc(sizeof(char));
                    }
                    else
                    {
                        if (line_case == START_OF_LINE && first_word)
                        {
                            word = check_words(word, str1, str2, mode);
                            word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                            strcpy(word_list.content[word_list.word_num], word);
                            word_list.word_num++;
                            word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                            word_len = 0;
                            free(word);
                            word = (char *)malloc(sizeof(char));
                            first_word = 0;
                        }
                        else if (line_case != START_OF_LINE)
                        {
                            word = check_words(word, str1, str2, mode);
                            word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                            strcpy(word_list.content[word_list.word_num], word);
                            word_list.word_num++;
                            word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                            word_len = 0;
                            free(word);
                            word = (char *)malloc(sizeof(char));
                        }
                        else
                        {
                            word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                            strcpy(word_list.content[word_list.word_num], word);
                            word_list.word_num++;
                            word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                            word_len = 0;
                            free(word);
                            word = (char *)malloc(sizeof(char));
                        }
                    }
                }
                // puts new line
                word_list.content[word_list.word_num] = (char *)malloc(2);
                strcpy(word_list.content[word_list.word_num], "\n");
                word_list.word_num++;
                word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                word_len = 0;
            }
            else if (ch == ' ')
            {
                if (word_len > 0)
                {
                    if (word_len != strlen(word))
                    {
                        word[word_len] = '\0';
                    }
                    if (line_case == START_OF_LINE && first_word)
                    {
                        word = check_words(word, str1, str2, mode);
                        word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                        strcpy(word_list.content[word_list.word_num], word);
                        word_list.word_num++;
                        word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                        word_len = 0;
                        free(word);
                        word = (char *)malloc(sizeof(char));
                        first_word = 0;
                    }
                    else if (start_line_flag == 1 && line_case == START_OF_LINE)
                    {
                        word = check_words(word, str1, str2, mode);
                        word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                        strcpy(word_list.content[word_list.word_num], word);
                        word_list.word_num++;
                        word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                        start_line_flag = 0;
                        word_len = 0;
                    }
                    else
                    {
                        if (line_case == NO_LINE_SYMBOL)
                        {
                            word = check_words(word, str1, str2, mode);
                            word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                            strcpy(word_list.content[word_list.word_num], word);
                            word_list.word_num++;
                            word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                            word_len = 0;
                            free(word);
                            word = (char *)malloc(sizeof(char));
                            word[0] = '\0';
                        }
                        else
                        {
                            word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
                            strcpy(word_list.content[word_list.word_num], word);
                            word_list.word_num++;
                            word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                            word_len = 0;
                        }
                    }
                    free(word);
                    word = (char *)malloc(sizeof(char));
                }
                // put blanks
                word_list.content[word_list.word_num] = (char *)malloc(2);
                strcpy(word_list.content[word_list.word_num], " ");
                word_list.word_num++;
                word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
                word_len = 0;
            }
            else
            {
                word[word_len] = ch;
                word_len++;
                word = (char *)realloc(word, word_len + 1);
            }
        }
    } // end of while

    if (start_line_flag == 1 && line_case == START_OF_LINE)
    {
        word = check_words(word, str1, str2, mode);
        word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
        strcpy(word_list.content[word_list.word_num], word);
        word_list.word_num++;
        word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
        word_len = 0;
        free(word);
        word = (char *)malloc(sizeof(char));
    }
    else if (line_case == END_OF_LINE)
    {
        word = check_words(word, str1, str2, mode);
        word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
        strcpy(word_list.content[word_list.word_num], word);
        word_list.word_num++;
        word_list.content = realloc(word_list.content, sizeof(char *) * word_list.word_num + 1);
        word_len = 0;
        free(word);
        word = (char *)malloc(sizeof(char));
    }
    else
    {
        word = check_words(word, str1, str2, mode);
        word_list.content[word_list.word_num] = (char *)malloc(strlen(word));
        strncpy(word_list.content[word_list.word_num], word, strlen(word));
        word_list.word_num++;
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    if (close(fd) == -1)
    {
        perror("close");
    }
    write_file(fpath, word_list);
    free(word_list.content);
    free(word);
}

int find_operation(char *arg)
{
    char *ret;
    if ((ret = strstr(arg, ";")))
        return MULTIPLE_OPERATION;
    else
        return SINGLE_OPERATION;
}

void write_file(char *file_path, Buffer text)
{

    struct flock lock;
    int fd;

    fd = open(file_path, O_TRUNC);
    if (fd == -1)
        perror("open");
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);

    if (close(fd) == -1)
        perror("close");

    fd = open(file_path, O_TRUNC || O_WRONLY);

    if (fd == -1)
        perror("open");
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lock);

    for (int i = 0; i < text.word_num; i++)
    {
        for (int j = 0; j < strlen(text.content[i]); j++)
        {
            if (write(fd, &text.content[i][j], 1) < 0)
            {
                perror("write");
            }
        }
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    if (close(fd) == -1)
        perror("close");
}
int find_case(char *arg)
{
    char *ret;
    if ((ret = strstr(arg, "i")))
        return CASE_INSENSITIVE;
    else
        return CASE_SENSITIVE;
}
int check_line(char *arg)
{
    char *ret;
    if ((ret = strstr(arg, "^")))
        return START_OF_LINE;
    else if ((ret = strstr(arg, "$")))
        return END_OF_LINE;
    else
        return CASE_SENSITIVE;
}

char *remove_symbols(char *string)
{
    char *temp = (char *)malloc(sizeof(string));
    strcpy(temp, string);
    int index;
    for (int i = 0; i < strlen(temp); i++)
    {
        if (temp[i] == '$' || temp[i] == '^')
        {
            index = i;
            for (int j = i + 1; j < strlen(temp); j++)
            {
                char ch = temp[index];
                temp[index] = temp[j];
                temp[j] = ch;
                index++;
            }
        }
        temp[index] = '\0';
    }
    strcpy(string, temp);
    return temp;
}

int check_alternative(char *string, char *expression, int mode)
{
    int multiple_char_flag; //[
    int one_or_more_flag;   // *
    int index = 0;
    int count = 0;
    char before_star;
    int equal_flag = 0;
    int i;
    char *multiple_chars = (char *)malloc(sizeof(char) * 1);
    for (i = 0; i < strlen(expression); i++)
    {
        one_or_more_flag = 0;
        multiple_char_flag = 0;
        count = 0;
        if (expression[i] == '[')
        {
            for (i += 1; i < strlen(expression) && expression[i] != ']'; i++)
            {
                multiple_chars[count] = expression[i];
                count++;
                multiple_chars = (char *)realloc((char *)multiple_chars, count);
            }
            multiple_char_flag = 1;
            i++;
        }
        if (expression[i] == '*')
        {
            before_star = expression[i - 1];
            one_or_more_flag = 1;
            i++;
        }
        if (one_or_more_flag == 1)
        {
            if (one_or_more_flag && multiple_char_flag)
            {
                equal_flag = 1;
                if (before_star == ']')
                {
                    for (int k = 0; k < count; k++)
                    {
                        before_star = multiple_chars[k];
                        if (check_chars(before_star, string[index], mode))
                        {
                            for (; index < strlen(string); index++)
                            {
                                if (check_chars(string[index], before_star, mode))
                                {
                                    continue;
                                }
                                else
                                {
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            else
            {
                equal_flag = 1;
                if (check_chars(before_star, string[index], mode))
                {
                    for (; index < strlen(string); index++)
                    {
                        if (check_chars(string[index], before_star, mode))
                        {
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
        else if (multiple_char_flag == 1)
        {
            int flag = 0;
            for (int k = 0; k < count; k++)
            {
                if (check_chars(string[index], multiple_chars[k], mode))
                {
                    flag = 1;
                }
                if (flag == 1)
                {
                    equal_flag = 1;
                    index++;
                    break;
                }
            }
            if (!flag)
            {
                equal_flag = 0;
            }
        }
        else
        {
            if (i + 1 < strlen(expression) && '*' == expression[i + 1])
            {
                continue;
            }
            else
            {
                if (check_chars(string[index], expression[i], mode))
                {

                    equal_flag = 1;
                }
                else
                {
                    equal_flag = 0;
                }
            }
        }
        if (equal_flag == 0)
        {
            break;
        }
        else
        {
            index++;
        }
    }
    free(multiple_chars);
    if (equal_flag == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char *check_words(char *word, char *expression, char *string, int mode)
{
    int flag = 0;
    int i;
    for (i = 0; i < strlen(word); i++)
    {
        if (check_alternative(word + i, expression, mode))
        {
            flag = 1;
            break;
        }
    }
    if (flag == 1)
    {
        word = realloc(word, strlen(string) + i);
        strncpy(word + i, string, strlen(string) + 1);
        flag = 0;
    }
    return word;
}

int check_input(char *string)
{
    int flag = 0;
    for (int i = 1; i < strlen(string); i++)
    {
        if (string[i] == '^')
        {
            perror("Wrong input format\n");
            return -1;
        }
    }
    for (int i = 1; i < strlen(string) - 1; i++)
    {
        if (string[i] == '$')
        {
            perror("Wrong input format\n");
            return -1;
        }
    }
    for (int i = 1; i < strlen(string); i++)
    {

        if (string[i] == '[')
        {
            flag = 1;
            for (int j = i; j < strlen(string); j++)
            {
                if (string[j] == ']')
                {
                    flag = 0;
                    break;
                }
            }
        }
    }
    if (flag == 1)
    {
        perror("Wrong input format\n");
        return -1;
    }
    return 0;
}