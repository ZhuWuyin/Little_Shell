#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "consts.h"

struct task
{
    unsigned int id;
    char *name;
    char **argv;
    pid_t pid;
    struct task *next;
} tasks = {0, NULL};

void append_task(char *program_path, char **argv, struct task *t){
    t->name = program_path;
    t->argv = argv;
    t->pid = fork();

    if (t->pid == -1){
        printf("Fail to do fork()\n");
    }
    else if (t->pid == 0){
        execv(program_path, argv);
        printf("Fail to load this program (execv())\n");
        exit(1);
    }
}

void clean_mem(char *program_path, char **argv){
    if (program_path != NULL){
        free(program_path);
    }
    if (argv != NULL){
        for (size_t i=0; argv[i]!=NULL; i++){
            free(argv[i]);
        }
        free(argv);
    }
}

void query_tasks(struct task *start){
    struct task *curr = start;
    while (curr != NULL){
        int status = 0;
        pid_t result = waitpid(curr->pid, &status, WNOHANG);
        char *str;
        if (result == 0 || curr->id == 0){
            str = "progressing";
        }
        else if (result == -1){
            str = "error";
        }
        else {
            str = "complete";
            clean_mem(NULL, curr->argv);
        }
        printf("[%u] %d %s - %s\n", curr->id, curr->pid, curr->name, str);
        curr = curr->next;
    }
}

int get_program_path(char *str, char *dest, size_t size){
    size_t end_i = 0;
    for (; end_i<strlen(str)-1 && str[end_i]!=' '; end_i++);
    if (end_i >= size){
        return -1;
    }

    for (int i=0; i<end_i; i++){
        dest[i] = str[i];
    }

    return end_i;
}

int args_count(char *str, size_t start_i){
    unsigned int counter = 0;
    size_t word_end = 0;
    for (; str[start_i] != '&'; start_i++){
        if (str[start_i] != ' '){
            word_end = start_i;
        }
        if (start_i - word_end == 1){
            counter++;
        }
    }
    if (word_end == 0){
        counter = 0;
    }
    return counter;
}

int get_arg(char *str, size_t start_i, char **dest, size_t arg_row, size_t arg_size){
    size_t end_i = start_i+1;
    for (; end_i<strlen(str)-1 && str[end_i]!=' '; end_i++);
    if (end_i - start_i >= arg_size){
        return -1;
    }

    for (size_t i=0; i<end_i-start_i; i++){
        dest[arg_row][i] = str[start_i+i];
    }
    dest[arg_row][end_i-start_i] = '\0';
    return end_i;
}

int get_program_args(char *str, size_t start_i, char **argv, size_t arg_len, size_t args_num){
    for (size_t i=0; i<args_num; i++){
        start_i = get_arg(str, start_i, argv, i, arg_len);
        for (; str[start_i]==' '; start_i++);
    }
    return 0;
}

struct task* run_task(char *str, struct task *curr){
    char *program_path = NULL;
    char **argv = NULL;
    size_t const program_path_len = DIR_LEN;

    program_path = (char*)malloc(program_path_len*(sizeof(char)));
    size_t start_i = get_program_path(str, program_path, program_path_len);
    int args_num = args_count(str, start_i);
    if (start_i == -1 || args_num >= ARGS_LEN || access(program_path, F_OK) == -1){
        goto err;
    }

    argv = (char**)malloc((args_num+1)*(sizeof(char*)));
    for (size_t i=0; i<(args_num+1); i++){
        argv[i] = (char*)malloc(ARG_LEN*(sizeof(char)));
    }
    argv[args_num] = NULL;
    for (; str[start_i]==' '; start_i++);
    if (get_program_args(str, start_i, argv, ARG_LEN, args_num)){
        goto err;
    }
    
    struct task *t = (struct task*)malloc(sizeof(struct task));
    t->next = NULL;
    t->id = curr->id+1;
    append_task(program_path, argv, t);
    return t;

    err:
        clean_mem(program_path, argv);
        printf("Wrong input\n");
        return NULL;
}