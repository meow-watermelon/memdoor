#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "network.h"

static char *tcp_state[] =
{
    "PLACEHOLDER",
    "ESTABLISHED",
    "SYN_SENT",
    "SYN_RECV",
    "FIN_WAIT1",
    "FIN_WAIT2",
    "TIME_WAIT",
    "CLOSE",
    "CLOSE_WAIT",
    "LAST_ACK",
    "LISTEN",
    "CLOSING"
};

void free_netstat(struct netstat *input_netstat) {
    struct netstat *current = input_netstat;
    struct netstat *next;

    while (current != NULL) {
        next = current->next_ptr;
        free(current);
        current = NULL;
        current = next;
    }
}

struct netstat *load_netstat(char *protocol) {
    char proc_netstat_filename[PATH_MAX];

    /* specify the network stat filename based on the protocol type */
    if (strcmp(protocol, "tcp") == 0) {
        strcpy(proc_netstat_filename, "/proc/net/tcp");
    } else if (strcmp(protocol, "udp") == 0) {
        strcpy(proc_netstat_filename, "/proc/net/udp");
    } else if (strcmp(protocol, "tcp6") == 0) {
        strcpy(proc_netstat_filename, "/proc/net/tcp6");
    } else if (strcmp(protocol, "udp6") == 0) {
        strcpy(proc_netstat_filename, "/proc/net/udp6");
    } else {
        fprintf(stderr, "ERROR: please pass correct protocol string: [tcp, tcp6, udp, udp6]\n");
        return NULL;
    }

    FILE *proc_netstat_file;
    proc_netstat_file = NULL;

    char line[BUFSIZ];

    /* define reading format of /proc/net/{tcp,udp,tcp6,udp6} file */
    char *format = "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %*s";

    /* define field variables of /proc/net/{tcp,tcp6,udp,udp6} file
     *
     * 1st: index (d)
     * 2nd: local address (64[0-9A-Fa-f])
     * 3rd: local port (X)
     * 4th: remote address (64[0-9A-Fa-f])
     * 5th: remote port (X)
     * 6th: socket state (X)
     * 7th: TX queue size (lX)
     * 8th: RX queue size (lX)
     * 9th: timer active (X)
     * 10th: number of jiffies until timer expires (lX)
     * 11th: number of unrecovered RTO timeouts (lX)
     * 12th: uid (d)
     * 13th: unanswered 0-window probes (d)
     * 14th: socket inode (ld)
     */
    int index;
    char local_address[128];
    int local_port;
    char remote_address[128];
    int remote_port;
    int socket_state;
    long int tx_queue;
    long int rx_queue;
    int timer_active;
    long int time_length;
    long int retry;
    int uid;
    int timeout;
    long int socket_inode;

    /* define address strings */
    struct in_addr ip_address;
    struct in6_addr ipv6_address;
    char local_address_string[INET_ADDRSTRLEN];
    char remote_address_string[INET_ADDRSTRLEN];
    char ipv6_local_address_string[INET6_ADDRSTRLEN];
    char ipv6_remote_address_string[INET6_ADDRSTRLEN];
    uint32_t local_address_hex;
    uint32_t remote_address_hex;

    int ret_sscanf;

    proc_netstat_file = fopen(proc_netstat_filename, "r");
    if (proc_netstat_file == NULL) {
        fprintf(stderr, "ERROR: failed to open %s stats file %s: %s\n", protocol, proc_netstat_filename, strerror(errno));
        return NULL;
    }

    /* build linked-list */
    struct netstat *head = NULL;
    struct netstat *next = NULL;

    while (fgets(line, sizeof(line), proc_netstat_file) != NULL) {
        /* read fields from each stat line */
        ret_sscanf = sscanf(line, format, &index, local_address, &local_port, remote_address, &remote_port, &socket_state, &tx_queue, &rx_queue, &timer_active, &time_length, &retry, &uid, &timeout, &socket_inode);

        if (ret_sscanf < 11) {
            continue;
        }

        /* IPv4 connections */
        if (strcmp(protocol, "tcp") == 0 || strcmp(protocol, "udp") == 0) {
            /* process local address and port*/
            ret_sscanf = sscanf(local_address, "%X", &local_address_hex);
            if (ret_sscanf < 1 || ret_sscanf == EOF) {
                continue;
            }

            ip_address.s_addr = local_address_hex;

            if (inet_ntop(AF_INET, &ip_address, local_address_string, INET_ADDRSTRLEN) == NULL) {
                continue;
            }

            /* process remote address and port*/
            ret_sscanf = sscanf(remote_address, "%X", &remote_address_hex);
            if (ret_sscanf < 1 || ret_sscanf == EOF) {
                continue;
            }

            ip_address.s_addr = remote_address_hex;

            if (inet_ntop(AF_INET, &ip_address, remote_address_string, INET_ADDRSTRLEN) == NULL) {
                continue;
            }
        }

        /* IPv6 connections */
        if (strcmp(protocol, "tcp6") == 0 || strcmp(protocol, "udp6") == 0) {
            /* process local address and port*/
            ret_sscanf = sscanf(local_address, "%08X%08X%08X%08X", &ipv6_address.s6_addr32[0], &ipv6_address.s6_addr32[1], &ipv6_address.s6_addr32[2], &ipv6_address.s6_addr32[3]);
            if (ret_sscanf < 4 || ret_sscanf == EOF) {
                continue;
            }

            if (inet_ntop(AF_INET6, &ipv6_address, ipv6_local_address_string, INET6_ADDRSTRLEN) == NULL) {
                continue;
            }

            /* process remote address and port*/
            ret_sscanf = sscanf(remote_address, "%08X%08X%08X%08X", &ipv6_address.s6_addr32[0], &ipv6_address.s6_addr32[1], &ipv6_address.s6_addr32[2], &ipv6_address.s6_addr32[3]);
            if (ret_sscanf < 4 || ret_sscanf == EOF) {
                continue;
            }

            if (inet_ntop(AF_INET6, &ipv6_address, ipv6_remote_address_string, INET6_ADDRSTRLEN) == NULL) {
                continue;
            }
        }

        /* create netstat struct */
        struct netstat *node = (struct netstat *)malloc(sizeof(struct netstat));
        if (node == NULL) {
            fprintf(stderr, "ERROR: failed to allocate memory for netstat struct\n");
            return NULL;
        }

        /* fill out the node */
        strcpy(node->protocol, protocol);
        node->socket_state = socket_state;
        node->socket_inode = socket_inode;

        if (strcmp(protocol, "tcp") == 0 || strcmp(protocol, "udp") == 0) {
            strcpy(node->local_address, local_address_string);
        }
        if (strcmp(protocol, "tcp6") == 0 || strcmp(protocol, "udp6") == 0) {
            strcpy(node->local_address, ipv6_local_address_string);
        }

        node->local_port = local_port;

        if (strcmp(protocol, "tcp") == 0 || strcmp(protocol, "udp") == 0) {
            strcpy(node->remote_address, remote_address_string);
        }
        if (strcmp(protocol, "tcp6") == 0 || strcmp(protocol, "udp6") == 0) {
            strcpy(node->remote_address, ipv6_remote_address_string);
        }

        node->remote_port = remote_port;
        node->tx_queue = tx_queue;
        node->rx_queue = rx_queue;
        node->next_ptr = NULL;

        if (head == NULL) {
            head = node;
            next = node;
        } else {
            next->next_ptr = node;
            next = node;
        }
    }

    fclose(proc_netstat_file);

    return head;
}

void get_connection_stats(long int input_socket_inode, struct netstat *input_netstat) {
    struct netstat *head = NULL;

    if (input_netstat == NULL) {
        return;
    }

    head = input_netstat;

    while (head != NULL) {
        if (head->socket_inode == input_socket_inode) {
            /* print network connection stats */
            fprintf(stdout, "%-6s%-13s%-45s%-8d%-45s%-8d%-10ld%-10ld\n", head->protocol, tcp_state[head->socket_state], head->local_address, head->local_port, head->remote_address, head->remote_port, head->tx_queue, head->rx_queue);
            fflush(stdout);
        }

        head = head->next_ptr;
    }
}
