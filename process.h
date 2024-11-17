#ifndef PROCESS_H
#define PROCESS_H

#include <limits.h>
#include <sys/types.h>

struct meminfo {
    int process_oom_score;
    int process_oom_score_adj;
    long int total_memory; /* unit: kB */
    long int process_rss; /* unit: kB */
    long int process_pss; /* unit: kB */
    long int process_uss; /* unit: kB */
};

extern int check_pid(pid_t pid);
extern int get_ppid(pid_t pid, int *ppid, char *exe_name);
extern char *get_exe_path_name(pid_t pid);
extern int compare_pid_exe(pid_t pid, char *exe_name);
extern int get_oom_score(pid_t pid, int *oom_score, int *oom_score_adj);
extern int get_memory_usage(pid_t pid, long int *process_rss, long int *process_pss, long int *process_uss);
extern int get_system_memory(long int *total_memory);
extern void get_process_tree(pid_t pid);
extern void get_memory_mapping(pid_t pid);
extern void get_network_connection(pid_t pid);

#endif /* PROCESS_H */
