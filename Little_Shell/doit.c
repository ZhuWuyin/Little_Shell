#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "consts.h"
#include "commands.c"
#include "sys_stats.c"

void sys_stats_print(clock_t start, clock_t end){
    printf("----Wall clock: %fms----\n", (double)(end - start)/CLOCKS_PER_SEC*1000);
    get_cpu_time_usage();
    get_preemption_and_relinquishment();
    get_page_faults();
    get_max_resident_set_size();
}

void __init__(struct task *start){
    start->argv = NULL;
    start->id = 0;
    start->name = "main";
    start->next = NULL;
    start->pid = getpid();
}

int main(int argc, char const *argv[])
{   
    char curr_dir[DIR_LEN];
    getcwd(curr_dir, sizeof(curr_dir));
    char prompt[PROMPT_LEN] = "==>";
    clock_t start = clock(), end = 0;
    if (argc == 3 && !strcmp(argv[1], "wc")){
        char const *file_path = argv[2];
        FILE *f = fopen(file_path, "r");
        wc(f);
        fclose(f);
        end = clock();
        sys_stats_print(start, end);
        return 0;
    }
    else if (argc == 3){
        printf("Wrong args\n");
        help();
        return 1;
    }
    
    struct task *start_task = (struct task*)malloc(sizeof(struct task));
    __init__(start_task);
    struct task *curr_task = start_task;
    while (true)
    {
        if (end != 0){
            sys_stats_print(start, end);
        }

        printf("%s", prompt);
        char buff[BUFF_LEN];
        fgets(buff, sizeof(buff), stdin);

        start = clock();
        char command[COMMAND_LEN];
        command[0] = '\0';
        size_t end_i = get_command(buff, command, sizeof(command));

        if (end_i == 0){
            struct task *end_task = run_task(buff, curr_task);
            if (end_task != NULL){
                curr_task->next = end_task;
                curr_task = end_task;
                query_tasks(start_task);
            }
            end = clock();
            continue;
        }

        for (; end_i<BUFF_LEN && buff[end_i]==' '; end_i++);
        if (!strcmp(command, "jobs")){
            query_tasks(start_task);
        }
        else if (!strcmp(command, "cat")){
            cat(buff, end_i, start_task);
        }
        else if (!strcmp(command, "ls")){
            ls(curr_dir);
        }
        else if (!strcmp(command, "cd")){
            cd(buff, end_i, curr_dir, sizeof(curr_dir));
        }
        else if (!strcmp(command, "date")){
            date();
        }
        else if (!strcmp(command, "set")){
            set(buff, end_i, prompt, sizeof(prompt));
        }
        else if (!strcmp(command, "exit")){
            break;
        }

        end = clock();
    }
    return 0;
}