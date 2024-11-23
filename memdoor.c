#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "process.h"
#include "utils.h"

#define VERSION "1.6.0"
#define PROCESS_BASIC_INFO_BANNER "##### PROCESS BASIC INFORMATION #####"
#define PROCESS_MEMORY_INFO_BANNER "##### PROCESS MEMORY INFORMATION #####"
#define PROCESS_TREE_INFO_BANNER "##### PROCESS TREE INFORMATION #####"
#define PROCESS_MEMORY_MAPPING_INFO_BANNER "##### PROCESS MEMORY MAPPING INFORMATION #####"
#define PROCESS_NETWORK_CONNECTION_INFO_BANNER "##### PROCESS NETWORK CONNECTION INFORMATION #####"

/* define command-line options */
char *short_opts = "p:e:m:i:c:";
struct option long_opts[] = {
    {"pid", required_argument, NULL, 'p'},
    {"exename", required_argument, NULL, 'e'},
    {"memory-pressure-threshold", required_argument, NULL, 'm'},
    {"interval", required_argument, NULL, 'i'},
    {"count", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}
};

/* define usage function */
static void usage() {
    printf(
        "memdoor version %s\n"
        "usage: memdoor -p|--pid <target process id>\n"
        "               -e|--exename <full path of target process>\n"
        "               -i|--interval <second(s)>\n"
        "               [-m|--memory-pressure-threshold <percentage integer>]\n"
        "               [-c|--count <count(s)>]\n", VERSION
    );
}

int main(int argc, char *argv[]) {
    pid_t pid;
    char exename[PATH_MAX];
    long int memory_pressure_threshold;
    long int interval;
    long int count = -1;

    struct meminfo memory_data;

    int ret_check_pid;
    int ret_compare_pid_exe;
    int ret_get_system_memory;
    int ret_get_memory_usage;
    int ret_get_page_tables_usage;
    int ret_get_oom_score;

    /* suppress default getopt error messages */
    opterr = 0;

    int c;

    int opt_flag_p = 0;
    int opt_flag_e = 0;
    int opt_flag_m = 0;
    int opt_flag_i = 0;

    while (1) {
        c = getopt_long (argc, argv, short_opts, long_opts, NULL);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'p':
                errno = 0;
                pid = strtol(optarg, NULL, 10);

                if (errno != 0) {
                    fprintf(stderr, "ERROR: failed to covert process ID value\n\n");
                    exit(EXIT_FAILURE);
                }

                if (pid <= 0) {
                    fprintf(stderr, "ERROR: process ID must be an integer and greater than 0\n\n");
                    usage();
                    exit(EXIT_FAILURE);
                }
                opt_flag_p = 1;
                break;
            case 'e':
                strncpy(exename, optarg, strlen(optarg) + 1);
                opt_flag_e = 1;
                break;
            case 'm':
                if (optarg != NULL) {
                    errno = 0;
                    memory_pressure_threshold = strtol(optarg, NULL, 10);

                    if (errno != 0) {
                        fprintf(stderr, "ERROR: failed to covert memory pressure threshold value\n\n");
                        exit(EXIT_FAILURE);
                    }

                    if (memory_pressure_threshold <= 0 || memory_pressure_threshold >= 100) {
                        fprintf(stderr, "ERROR: memory pressure threshold must be an integer and the range should be [1,99]\n\n");
                        usage();
                        exit(EXIT_FAILURE);
                    }
                    opt_flag_m = 1;
                }
                break;
            case 'i':
                errno = 0;
                interval = strtol(optarg, NULL, 10);

                if (errno != 0) {
                    fprintf(stderr, "ERROR: failed to covert interval value\n\n");
                    exit(EXIT_FAILURE);
                }

                if (interval <= 0) {
                    fprintf(stderr, "ERROR: interval must be an integer and greater than 0\n\n");
                    usage();
                    exit(EXIT_FAILURE);
                }
                opt_flag_i = 1;
                break;
            case 'c':
                if (optarg != NULL) {
                    errno = 0;
                    count = strtol(optarg, NULL, 10);

                    if (errno != 0) {
                        fprintf(stderr, "ERROR: failed to covert count value\n\n");
                        exit(EXIT_FAILURE);
                    }

                    if (count <= 0) {
                        fprintf(stderr, "ERROR: count must be an integer and greater than 0\n\n");
                        usage();
                        exit(EXIT_FAILURE);
                    }
                } else {
                    /* use default count vaule as loop flag */
                    count = -1;
                }
                break;
            case '?':
                fprintf(stderr, "ERROR: Unknown option\n\n");
                usage();
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "ERROR: Unimplemented option\n\n");
                usage();
                exit(EXIT_FAILURE);
        }
    }

    /* check if necessary options are specified */
    if (!opt_flag_p || !opt_flag_e || !opt_flag_i) {
        usage();
        exit(EXIT_FAILURE);
    }

    while (1) {
        /* exit the loop once count becomes 0 */
        if (count == 0) {
            break;
        }

        /* print timestamp */
        print_current_time();

        /* check if PID exists and have permission to read information */
        ret_check_pid = check_pid(pid);
        if (ret_check_pid != 0) {
            fprintf(stderr, "ERROR: PID %d is not accessible: %s\n", pid, strerror(ret_check_pid));
            exit(EXIT_FAILURE);
        }

        /* check if PID matches the input executable absolute path */
        ret_compare_pid_exe = compare_pid_exe(pid, exename);
        if (ret_compare_pid_exe < 0) {
            fprintf(stderr, "ERROR: PID %d does not match the executable name %s\n", pid, exename);
            exit(EXIT_FAILURE);
        }

        /* print process basic information */
        fprintf(stdout, "%s\n", PROCESS_BASIC_INFO_BANNER);
        fprintf(stdout, "PID: %d\n", pid);
        fprintf(stdout, "Executable Absolute Path: %s\n\n", exename);
        fflush(stdout);

        /* check if process memory usage is equal or greater than input memory pressure threshold */
        ret_get_system_memory = get_system_memory(&memory_data.total_memory);
        if (ret_get_system_memory < 0) {
            fprintf(stderr, "ERROR: failed to get system memory information\n\n");
            sleep(interval);

            if (count > 0) {
                --count;
            }

            continue;
        }

        /* get process memory usage */
        ret_get_memory_usage = get_memory_usage(pid, &memory_data.process_rss, &memory_data.process_pss, &memory_data.process_uss);
        if (ret_get_memory_usage < 0) {
            fprintf(stderr, "ERROR: failed to get process memory usage information\n\n");
            sleep(interval);

            if (count > 0) {
                --count;
            }

            continue;
        }

        /* get process page tables usage */
        ret_get_page_tables_usage = get_page_tables_usage(pid, &memory_data.process_page_tables_size);
        if (ret_get_page_tables_usage < 0) {
            fprintf(stderr, "ERROR: failed to get process page tables usage information\n\n");
            sleep(interval);

            if (count > 0) {
                --count;
            }

            continue;
        }

        if (opt_flag_m == 1) {
            if ((int)((float)memory_data.process_rss / (float)memory_data.total_memory * 100) < memory_pressure_threshold) {
                fprintf(stdout, "Process memory usage is not equal to or greater than input memory pressure threshold\n\n");
                fflush(stdout);
                sleep(interval);

                if (count > 0) {
                    --count;
                }

                continue;
            }
        }

        /* print process memory and page tables usage information */
        fprintf(stdout, "%s\n", PROCESS_MEMORY_INFO_BANNER);
        fflush(stdout);

        fprintf(stdout, "Total System Memory: %ld kB\n", memory_data.total_memory);
        fprintf(stdout, "Process RSS Memory Usage: %ld kB\n", memory_data.process_rss);
        fprintf(stdout, "Process PSS Memory Usage: %ld kB\n", memory_data.process_pss);
        fprintf(stdout, "Process USS Memory Usage: %ld kB\n", memory_data.process_uss);
        fprintf(stdout, "Process Page Tables Usage: %ld kB\n", memory_data.process_page_tables_size);
        fflush(stdout);

        ret_get_oom_score = get_oom_score(pid, &memory_data.process_oom_score, &memory_data.process_oom_score_adj);
        if (ret_get_oom_score < 0) {
            fprintf(stderr, "WARNING: failed to get process OOM score\n");
        } else {
            fprintf(stdout, "Process OOM Score: %d\n", memory_data.process_oom_score);
            fprintf(stdout, "Process OOM Score Adjustment Value: %d\n", memory_data.process_oom_score_adj);
            fflush(stdout);
        }

        fprintf(stdout, "\n");
        fflush(stdout);

        /* print process tree information */
        fprintf(stdout, "%s\n", PROCESS_TREE_INFO_BANNER);
        fflush(stdout);

        get_process_tree(pid);

        fprintf(stdout, "\n");
        fflush(stdout);

        /* print process memory mapping information */
        fprintf(stdout, "%s\n", PROCESS_MEMORY_MAPPING_INFO_BANNER);
        fflush(stdout);

        get_memory_mapping(pid);

        fprintf(stdout, "\n");
        fflush(stdout);
        
        /* print process network connection information */
        fprintf(stdout, "%s\n", PROCESS_NETWORK_CONNECTION_INFO_BANNER);
        fflush(stdout);

        get_network_connection(pid);

        fprintf(stdout, "\n");
        fflush(stdout);

        sleep(interval);            
        fprintf(stdout, "\n");
        fflush(stdout);

        if (count > 0) {
            --count;
        }
    }

    exit(EXIT_SUCCESS);
}
