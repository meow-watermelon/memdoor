#ifndef PROCESS_H
#define PROCESS_H

struct meminfo {
    int process_oom_score;
    int process_oom_score_adj;
    long int total_memory; /* unit: kB */
    long int process_rss; /* unit: kB */
    long int process_pss; /* unit: kB */
};

extern int check_pid(pid_t pid);
extern int compare_pid_exe(pid_t, char *exe_name);
extern int get_oom_score(pid_t pid, struct meminfo *input_meminfo);
extern int get_memory_usage(pid_t pid, struct meminfo *input_meminfo);
extern int get_system_memory(struct meminfo *input_meminfo);
extern void get_memory_mapping(pid_t pid);
extern void get_network_connection(pid_t pid);

#endif /* PROCESS_H */
