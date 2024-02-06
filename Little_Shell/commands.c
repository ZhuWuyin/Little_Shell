#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include "consts.h"
#include "bg_task.c"

char const *commands[] = {"cat", "jobs", "ls", "cd", "date", "set", "exit"};
size_t const commands_len = 7;

void ls(char *curr_dir){
    DIR *dir = opendir(curr_dir);
    if (dir == NULL){
        printf("Something went wrong, try again\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        printf("%s\n", entry->d_name);
    }
    
    closedir(dir);
}

void wc(FILE *f){
    if (f == NULL){
        printf("Unable to open this file\n");
        return;
    }

    unsigned int bytes = 0;
    unsigned int words = 0;
    unsigned int lines = 0;

    int character;
    size_t index = 0;
    size_t word_end = 0;
    while ((character = fgetc(f)) != EOF)
    {
        index++;
        bytes++;
        if (character != ' ' && character != '\n'){
            word_end = index;
        }
        if (index - word_end == 1){
            words++;
        }
        if (character == '\n'){
            lines++;
        }
        printf("%c", character);
    }

    if (bytes != 0){
        lines++;
    }
    if (word_end == 0){
        words = 0;
    }
    else if (index == word_end){
        words++;
    }

    printf("\nbytes: %u; words: %u; lines: %u;\n", bytes, words, lines);
}

void cat(char *str, size_t start_i, struct task *start){
    char file_path[DIR_LEN];
    size_t str_len = strlen(str) - 1;
    size_t path_len = str_len - start_i;
    if (path_len >= DIR_LEN){
        printf("Path is too long\n");
        return;
    }

    for (size_t i=0; i<path_len; i++){
        file_path[i] = str[start_i+i];
    }
    file_path[path_len] = '\0';

    query_tasks(start);
    FILE *f = fopen(file_path, "r");
    wc(f);
    fclose(f);
}

void cd(char *str, size_t start_i, char *curr_dir, size_t size){
    size_t str_len = strlen(str) - 1;
    size_t dir_len = str_len-start_i;
    if (dir_len >= size){
        goto err;
    }

    char dest[DIR_LEN];
    for (size_t i=0; i<dir_len; i++){
        dest[i] = str[start_i+i];
    }
    dest[dir_len] = '\0';

    if (chdir(dest)){
        goto err;
    }
    getcwd(curr_dir, size);
    return;

    err:
        printf("Fail to change directory\n");
}

void date(){
    time_t curr_time;
    time(&curr_time);
    struct tm *local_time = localtime(&curr_time);

    int year = local_time->tm_year + 1900;
    int month = local_time->tm_mon + 1;
    int day = local_time->tm_mday;
    int hour = local_time->tm_hour;
    int minute = local_time->tm_min;
    int second = local_time->tm_sec;

    printf("Date-Time: %02d/%02d/%04d - %02d:%02d:%02d\n", month, day, year, hour, minute, second);
}

void help(){
    printf(\
"\
\n==>date\n\
    <print the current date and time>\n\
    <statistics about this date command>\n\
==>cd dir\n\
    <current directory is changed to dir (if successful)>\n\
==>ls\n\
    <listing of files in the current directory>\n\
    <statistics about this 'ls' command>\n\
==>set prompt=myprompt:\n\
    <change string used for your shell prompt>\n\
myprompt:exit\n\
    <back to the shell prompt>\n\
"\
    );
}

void set(char *str, size_t start_i, char *dest, size_t size){
    size_t str_len = strlen(str) - 1;
    size_t args_len = str_len - start_i;
    if (args_len >= ARGS_LEN || args_len == 0){
        goto err;
    }

    char key[ARGS_LEN];
    int i = 0;
    for (; i<args_len && str[start_i+i]!=' ' && str[start_i+i]!='='; i++){
        key[i] = str[start_i+i];
    }
    key[i] = '\0';

    for (; start_i<str_len && str[start_i]!='='; start_i++);
    start_i++;
    for (; start_i<str_len && str[start_i]==' '; start_i++);

    char value[PROMPT_LEN];
    size_t value_len = str_len - start_i;
    for (i=0; i<value_len && start_i<str_len; i++){
        value[i] = str[start_i+i];
    }
    value[i] = '\0';

    if (!strcmp(key, "prompt")){
        size_t myprompt_len = strlen(value);
        if (myprompt_len >= size || myprompt_len == 0){
            printf("Please make a common prompt\n");
            return;
        }

        for (int i=0; i<=myprompt_len; i++){
            dest[i] = value[i];
        }
        return;
    }

    err:
        printf("Wrong args\n");
}

size_t get_command(char *str, char *dest, size_t size){
    size_t str_len = strlen(str) - 1;

    if (str[str_len - 1] == '&' && str[str_len - 2] == ' '){
        return 0;
    }

    size_t end_i = 0;
    for (; end_i<str_len && str[end_i] != ' '; end_i++);
    if (end_i >= size){
        goto err;
    }

    char command[COMMAND_LEN];
    for (size_t i=0; i<end_i; i++){
        command[i] = str[i];
    }
    command[end_i] = '\0';

    size_t command_i = 0;
    for (; command_i<commands_len && strcmp(commands[command_i], command); command_i++);
    if (command_i == commands_len){
        goto err;
    }

    strcpy(dest, command);
    return end_i;

    err:
        printf("Wrong command\n");
        help();
        return -1;
}