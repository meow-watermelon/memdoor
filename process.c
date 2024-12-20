#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include "process.h"
#include "network.h"

int check_pid(pid_t pid) {
    int ret_kill;

    /* send null signal */
    ret_kill = kill(pid, 0);

    if (ret_kill == 0) {
        return 0;
    } else {
        return errno;
    }
}

int get_ppid(pid_t pid, int *ppid, char *exe_name) {
    FILE *pid_stat_file;
    pid_stat_file = NULL;
    char pid_stat_path[PATH_MAX];
    char *stat_format = "%*d %s %*s %d";

    int ret_snprintf;
    int ret_fscanf;

    /* construct /proc/pid/stat file path name */
    ret_snprintf = snprintf(pid_stat_path, sizeof(pid_stat_path), "/proc/%d/stat", pid);
    if (ret_snprintf < 0) {
        return -1;
    }

    /* get parent PID */
    pid_stat_file = fopen(pid_stat_path, "r");
    if (pid_stat_file == NULL) {
        return -1;
    }

    ret_fscanf = fscanf(pid_stat_file, stat_format, exe_name, ppid);

    if (ret_fscanf < 2 || ret_fscanf == EOF) {
        fclose(pid_stat_file);
        return -1;
    }

    fclose(pid_stat_file);

    return 0;
}

char *get_exe_path_name(pid_t pid) {
    int ret_snprintf;
    char exe_name_path[PATH_MAX];
    char *exe_name_realpath = (char *)malloc(PATH_MAX);

    if (exe_name_realpath == NULL) {
        return NULL;
    }

    /* construct /proc/pid/exe file path name */
    ret_snprintf = snprintf(exe_name_path, sizeof(exe_name_path), "/proc/%d/exe", pid);
    if (ret_snprintf < 0) {
        goto handle_error;
    }

    /* acquire real absolute path */
    if (realpath(exe_name_path, exe_name_realpath) == NULL) {
        goto handle_error;
    }

    return exe_name_realpath;

/* error handling routine */
handle_error:
    free(exe_name_realpath);
    exe_name_realpath = NULL;
    return NULL;
}

int compare_pid_exe(pid_t pid, char *exe_name) {
    int ret_strcmp;
    char *exe_name_realpath;
    exe_name_realpath = get_exe_path_name(pid);

    if (exe_name_realpath == NULL) {
        return -1;
    }

    /* compare executable absolute path and executable real path */
    ret_strcmp = strcmp(exe_name, exe_name_realpath);
    free(exe_name_realpath);
    exe_name_realpath = NULL;

    if (ret_strcmp == 0) {
        return 0;
    } else {
        return -1;
    }
}

int get_oom_score(pid_t pid, int *oom_score, int *oom_score_adj) {
    FILE *oom_score_file;
    FILE *oom_score_adj_file;
    oom_score_file = NULL;
    oom_score_adj_file = NULL;

    char oom_score_file_path[PATH_MAX];
    char oom_score_adj_file_path[PATH_MAX];

    /* set fail-safe values for oom_score and oom_score_adj*/
    *oom_score = -1;
    *oom_score_adj = -9999;

    int ret_snprintf;

    /* construct oom score file paths based on pid */
    ret_snprintf = snprintf(oom_score_file_path, sizeof(oom_score_file_path), "/proc/%d/oom_score", pid);
    if (ret_snprintf < 0) {
        return -1;
    }

    ret_snprintf = snprintf(oom_score_adj_file_path, sizeof(oom_score_adj_file_path), "/proc/%d/oom_score_adj", pid);
    if (ret_snprintf < 0) {
        return -1;
    }

    /* read oom_score */
    oom_score_file = fopen(oom_score_file_path, "r");
    if (oom_score_file == NULL) {
        goto handle_error;
    }

    if (fscanf(oom_score_file, "%d", oom_score) != 1) {
        goto handle_error;
    }

    /* read oom_score_adj */
    oom_score_adj_file = fopen(oom_score_adj_file_path, "r");
    if (oom_score_adj_file == NULL) {
        goto handle_error;
    }

    if (fscanf(oom_score_adj_file, "%d", oom_score_adj) != 1) {
        goto handle_error;
    }

    /* close file handles */
    fclose(oom_score_file);
    fclose(oom_score_adj_file);

    return 0;

/* error handling routine */
handle_error:
    if (oom_score_file != NULL) {
        fclose(oom_score_file);
    }

    if (oom_score_adj_file != NULL) {
        fclose(oom_score_adj_file);
    }

    return -1;
}

int get_system_memory(long int *total_memory) {
    char *system_meminfo_filename = "/proc/meminfo";
    FILE *system_meminfo_file;
    system_meminfo_file = NULL;

    char line[BUFSIZ];
    char *toggle_str;
    *total_memory = -1;

    system_meminfo_file = fopen(system_meminfo_filename, "r");
    if (system_meminfo_file == NULL) {
        return -1;
    }

    /* locate MemTotal string in /proc/meminfo file */
    while (fgets(line, sizeof(line), system_meminfo_file) != NULL) {
        if (strstr(line, "MemTotal:") != NULL) {
            /* skip ':' char */
            toggle_str = strchr(line, ':') + 1;

            /* covert the string to integer */
            errno = 0;
            *total_memory = strtol(toggle_str, NULL, 10);

            if (errno != 0) {
                fprintf(stderr, "ERROR: failed to convert MemTotal value\n");
            }

            break;
        }
    }

    fclose(system_meminfo_file);

    /* if total_memory is not equal to -1, that means we have acquired total memory value. otherwise, return -1 as error */
    if (*total_memory != -1) {
        return 0;
    } else {
        return -1;
    }
}

int get_memory_usage(pid_t pid, long int *process_rss, long int *process_pss, long int *process_uss) {
    FILE *process_smaps_rollup_file;
    process_smaps_rollup_file = NULL;

    char process_smaps_rollup_file_path[PATH_MAX];
    int ret_snprintf;

    char line[BUFSIZ];
    char *toggle_str;

    int success_flag = 0;

    /* USS = Private_Clean + Private_Dirty */
    long int uss_private_clean = -1;
    long int uss_private_dirty = -1;

    *process_rss = -1;
    *process_pss = -1;
    *process_uss = -1;

    /* construct process smaps_rollup file path based on pid */
    ret_snprintf = snprintf(process_smaps_rollup_file_path, sizeof(process_smaps_rollup_file_path), "/proc/%d/smaps_rollup", pid);
    if (ret_snprintf < 0) {
        return -1;
    }

    process_smaps_rollup_file = fopen(process_smaps_rollup_file_path, "r");
    if (process_smaps_rollup_file == NULL) {
        return -1;
    }

    /* locate Rss / Pss / Private_* string in /proc/pid/smaps_rollup file */
    while (fgets(line, sizeof(line), process_smaps_rollup_file) != NULL) {
        if (strstr(line, "Rss:") != NULL) {
            /* skip ':' char */
            toggle_str = strchr(line, ':') + 1;

            /* covert the string to integer */
            errno = 0;
            *process_rss = strtol(toggle_str, NULL, 10);

            if (errno != 0) {
                fprintf(stderr, "ERROR: failed to convert Rss value\n");
                *process_rss = -1;
            }
        }

        if (strstr(line, "Pss:") != NULL) {
            /* skip ':' char */
            toggle_str = strchr(line, ':') + 1;

            /* covert the string to integer */
            errno = 0;
            *process_pss = strtol(toggle_str, NULL, 10);

            if (errno != 0) {
                fprintf(stderr, "ERROR: failed to convert Pss value\n");
                *process_pss = -1;
            }
        }

        if (strstr(line, "Private_Clean:") != NULL) {
            /* skip ':' char */
            toggle_str = strchr(line, ':') + 1;

            /* covert the string to integer */
            errno = 0;
            uss_private_clean = strtol(toggle_str, NULL, 10);

            if (errno != 0) {
                fprintf(stderr, "ERROR: failed to convert Private_Clean value\n");
                uss_private_clean = -1;
            }
        }

        if (strstr(line, "Private_Dirty:") != NULL) {
            /* skip ':' char */
            toggle_str = strchr(line, ':') + 1;

            /* covert the string to integer */
            errno = 0;
            uss_private_dirty = strtol(toggle_str, NULL, 10);

            if (errno != 0) {
                fprintf(stderr, "ERROR: failed to convert Private_Dirty value\n");
                uss_private_dirty = -1;
            }
        }

        /* exit the loop once Pss, Rss, Private_Clean and Private_Dirty are found */
        if (*process_rss != -1 && *process_pss != -1 && uss_private_clean != -1 && uss_private_dirty != -1) {
            success_flag = 1;
            break;
        }
    }

    fclose(process_smaps_rollup_file);

    /* if neither of process_rss, process_pss and process_uss is not equal to -1, that means we have acquired process rss, pss and uss values. otherwise, return -1 as error */
    if (success_flag == 1) {
        /* calculate uss */
        *process_uss = uss_private_clean + uss_private_dirty;

        return 0;
    } else {
        return -1;
    }
}

int get_page_tables_usage(pid_t pid, long int *process_page_tables_size) {
    FILE *process_status_file;
    process_status_file = NULL;

    char process_status_file_path[PATH_MAX];
    int ret_snprintf;

    char line[BUFSIZ];
    char *toggle_str;

    *process_page_tables_size = -1;

    /* construct process status file path based on pid */
    ret_snprintf = snprintf(process_status_file_path, sizeof(process_status_file_path), "/proc/%d/status", pid);
    if (ret_snprintf < 0) {
        return -1;
    }

    process_status_file = fopen(process_status_file_path, "r");
    if (process_status_file == NULL) {
        return -1;
    }

    /* locate VmPTE string in /proc/pid/status file */
    while (fgets(line, sizeof(line), process_status_file) != NULL) {
        if (strstr(line, "VmPTE:") != NULL) {
            /* skip ':' char */
            toggle_str = strchr(line, ':') + 1;

            /* covert the string to integer */
            errno = 0;
            *process_page_tables_size = strtol(toggle_str, NULL, 10);

            if (errno != 0) {
                fprintf(stderr, "ERROR: failed to convert VmPTE value\n");
                *process_page_tables_size = -1;
            }

            break;
        }
    }

    fclose(process_status_file);

    /* if process_page_tables_size is not equal to -1, that means we have acquired page tables size value. otherwise, return -1 as error */
    if (*process_page_tables_size != -1) {
        return 0;
    } else {
        return -1;
    }
}

void get_process_tree(pid_t pid) {
    pid_t ppid;
    ppid = -1;

    pid_t tmp_pid;
    int oom_score;
    int oom_score_adj;
    long int process_rss;
    long int process_pss;
    long int process_uss;
    char exe_name[BUFSIZ];

    int ret_get_ppid;

    tmp_pid = pid;

    while (ppid != 0) {
        ret_get_ppid = get_ppid(tmp_pid, &ppid, exe_name);
        if (ret_get_ppid < 0) {
            fprintf(stderr, "WARNING: failed to get parent PID of the PID %d\n", tmp_pid);
            return;
        }

        get_oom_score(tmp_pid, &oom_score, &oom_score_adj);
        get_memory_usage(tmp_pid, &process_rss, &process_pss, &process_uss);

        /* print process tree in reverse order */
        printf("%d %s - OOM score: %d - OOM adjustment score: %d - RSS: %ld kB - PSS: %ld kB - USS: %ld kB\n", tmp_pid, exe_name, oom_score, oom_score_adj, process_rss, process_pss, process_uss);

        tmp_pid = ppid;
    }
}

void get_memory_mapping(pid_t pid) {
    FILE *process_memory_mapping_file;
    process_memory_mapping_file = NULL;

    char process_memory_mapping_file_path[PATH_MAX];

    int ret_sscanf;
    int ret_snprintf;

    char line[BUFSIZ];

    /* define reading format of /proc/pid/maps file */
    char *format = "%lx-%lx %4s %lx %5s %ld %s";

    /* define field variables of /proc/pid/maps file
     *
     * 1st: start address (lx)
     * 2nd: end address (lx)
     * 3rd: memory permission bits (4s)
     * 4th: offset (lx)
     * 5th: device major and minor (5s)
     * 6th: mapping file inode (ld)
     * 7th: mapping file pathname (s)
     */
    long int start_address;
    long int end_address;
    char permission_bits[5];
    long int offset;
    char dev[6];
    long int file_inode;

    char file_pathname[PATH_MAX];

    /* define memory usage footprint for each mapping */
    long int size;

    /* construct process memory mapping file path based on pid */
    ret_snprintf = snprintf(process_memory_mapping_file_path, sizeof(process_memory_mapping_file_path), "/proc/%d/maps", pid);
    if (ret_snprintf < 0) {
        fprintf(stderr, "ERROR: failed to construct the PID %d memory mapping file name\n", pid);
        return;
    }

    process_memory_mapping_file = fopen(process_memory_mapping_file_path, "r");
    if (process_memory_mapping_file == NULL) {
        fprintf(stderr, "ERROR: failed to open the PID %d memory mapping file: %s\n", pid, process_memory_mapping_file_path);
        return;
    }

    /* print header */
    fprintf(stdout, "%-16s  %-15s     %-5s %-6s %-12s %s\n", "START ADDRESS", "SIZE", "PERM", "DEV", "INODE", "FILE PATH");
    fflush(stdout);

    while (fgets(line, sizeof(line), process_memory_mapping_file) != NULL) {
        /* it is possible that pathname field is empty, set file_pathname as an empty string first as placeholder */
        file_pathname[0] = '\0';

        /* read fields from each mapping */
        ret_sscanf = sscanf(line, format, &start_address, &end_address, permission_bits, &offset, dev, &file_inode, file_pathname);
        if (ret_sscanf < 6 || ret_sscanf == EOF) {
            continue;
        }

        /* calculate virtual memory usage */
        size = end_address - start_address;

        /* print memory mappings */
        fprintf(stdout, "%016lx  %-15ld kB  %-5s %-6s %-12ld %s\n", start_address, size / 1024, permission_bits, dev, file_inode, file_pathname);
        fflush(stdout);
    }

    fclose(process_memory_mapping_file);
}

void get_network_connection(pid_t pid) {
    DIR *process_fd_dir;
    char process_fd_path[PATH_MAX];
    struct dirent *entry;
    char fd_file_symlink_path[PATH_MAX];
    char fd_file_path[PATH_MAX];

    int ret_readlink;
    int ret_snprintf;
    int ret_sscanf;

    long int socket_inode = -1;

    /* construct process file descriptors holding path based on pid */
    ret_snprintf = snprintf(process_fd_path, sizeof(process_fd_path), "/proc/%d/fd", pid);
    if (ret_snprintf < 0) {
        fprintf(stderr, "ERROR: failed to construct the PID %d memory mapping file name\n", pid);
        return;
    }

    /* read /proc/pid/fd directory */
    process_fd_dir = opendir(process_fd_path);
    if (process_fd_dir == NULL) {
        return;
    }

    /* load tcp and udp netstat data */
    struct netstat *tcp_netstat = load_netstat("tcp");
    if (tcp_netstat == NULL) {
        fprintf(stderr, "ERROR: failed to load IPv4 TCP network connections stats\n");
        return;
    }

    struct netstat *udp_netstat = load_netstat("udp");
    if (udp_netstat == NULL) {
        fprintf(stderr, "ERROR: failed to load IPv4 UDP network connections stats\n");
        return;
    }

    struct netstat *tcp6_netstat = load_netstat("tcp6");
    if (tcp6_netstat == NULL) {
        fprintf(stderr, "WARNING: failed to load IPv6 TCP network connections stats\n");
    }

    struct netstat *udp6_netstat = load_netstat("udp6");
    if (udp6_netstat == NULL) {
        fprintf(stderr, "WRANING: failed to load IPv6 UDP network connections stats\n");
    }

    /* print header */
    fprintf(stdout, "%-6s%-13s%-45s%-8s%-45s%-8s%-10s%-10s\n", "PROT", "STATE", "L.ADDR", "L.PORT", "R.ADDR", "R.PORT", "TX QUEUE", "RX QUEUE");
    fflush(stdout);

    while ((entry = readdir(process_fd_dir)) != NULL) {
        /* skip . and .. */
        if (entry->d_name[0] == '.') {
            continue;
        }

        /* construct file descriptor symlink file path */
        ret_snprintf = snprintf(fd_file_symlink_path, sizeof(fd_file_symlink_path), "/proc/%d/fd/%s", pid, entry->d_name);
        if (ret_snprintf < 0) {
            continue;
        }

        /* acquire real path */
        ret_readlink = readlink(fd_file_symlink_path, fd_file_path, sizeof(fd_file_path));
        if (ret_readlink < 0) {
            continue;
        }

        /* readlink() does not append null byte in the end of buffer */
        fd_file_path[ret_readlink] = '\0';

        /* acquire socket inode */
        ret_sscanf = sscanf(fd_file_path, "socket:[%ld]", &socket_inode);
        if (ret_sscanf < 1 || ret_sscanf == EOF) {
            continue;
        }

        /* we only process network connection details if socket_inode > 0 */
        if (socket_inode > 0) {
            /* process IPv4 TCP connection information */
            get_connection_stats(socket_inode, tcp_netstat);

            /* process IPv4 UDP connection information */
            get_connection_stats(socket_inode, udp_netstat);

            /* we only process IPv6 connection information if IPv6 is enabled */
            if (tcp6_netstat != NULL) {
                get_connection_stats(socket_inode, tcp6_netstat);
            }

            if (udp6_netstat != NULL) {
                get_connection_stats(socket_inode, udp6_netstat);
            }
        }
    }

    /* free linked list */
    free_netstat(tcp_netstat);
    free_netstat(udp_netstat);
    free_netstat(tcp6_netstat);
    free_netstat(udp6_netstat);

    closedir(process_fd_dir);
}
