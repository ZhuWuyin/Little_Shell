#include <sys/resource.h>

void get_cpu_time_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("----User CPU time: %ld milliseconds----\n", usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000);
    printf("----System CPU time: %ld milliseconds----\n", usage.ru_stime.tv_sec * 1000 + usage.ru_stime.tv_usec / 1000);
}

void get_preemption_and_relinquishment() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("----Number of involuntary preemptions: %ld----\n", usage.ru_nivcsw);
    printf("----Number of voluntary relinquishments: %ld----\n", usage.ru_nvcsw);
}

void get_page_faults() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("----Number of major page faults: %ld----\n", usage.ru_majflt);
    printf("----Number of minor page faults: %ld----\n", usage.ru_minflt);
}

void get_max_resident_set_size() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("----Maximum resident set size: %ldkb----\n", usage.ru_maxrss);
}