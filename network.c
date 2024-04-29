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

void get_tcp_connection_stats(long int input_socket_inode) {
    char *proc_ipv4_tcp_stat_filename = "/proc/net/tcp";
    FILE *proc_ipv4_tcp_stat_file;
    proc_ipv4_tcp_stat_file = NULL;

    char line[BUFSIZ];

    /* define reading format of /proc/net/tcp file */
    char *format = "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %*s";

    /* define field variables of /proc/net/tcp file
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
    char local_address_string[INET_ADDRSTRLEN];
    char remote_address_string[INET_ADDRSTRLEN];
    uint32_t local_address_hex;
    uint32_t remote_address_hex;

    int ret_sscanf;

    proc_ipv4_tcp_stat_file = fopen(proc_ipv4_tcp_stat_filename, "r");
    if (proc_ipv4_tcp_stat_file == NULL) {
        fprintf(stderr, "ERROR: failed to open IPv4 TCP stats file %s: %s\n", proc_ipv4_tcp_stat_filename, strerror(errno));
        return;
    }

    while (fgets(line, sizeof(line), proc_ipv4_tcp_stat_file) != NULL) {
        /* read fields from each stat line */
        ret_sscanf = sscanf(line, format, &index, local_address, &local_port, remote_address, &remote_port, &socket_state, &tx_queue, &rx_queue, &timer_active, &time_length, &retry, &uid, &timeout, &socket_inode);

        if (ret_sscanf < 11) {
            continue;
        }

        if (input_socket_inode == socket_inode) {
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

            /* print network connection stats */
            fprintf(stdout, "TCP  %-13s%s:%d  %s:%d  %ld  %ld\n", tcp_state[socket_state], local_address_string, local_port, remote_address_string, remote_port, tx_queue, rx_queue);
            fflush(stdout);
        }
    }

    fclose(proc_ipv4_tcp_stat_file);
}

void get_udp_connection_stats(long int input_socket_inode) {
    char *proc_ipv4_udp_stat_filename = "/proc/net/udp";
    FILE *proc_ipv4_udp_stat_file;
    proc_ipv4_udp_stat_file = NULL;

    char line[BUFSIZ];

    /* define reading format of /proc/net/udp file */
    char *format = "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %ld %*s";

    /* define field variables of /proc/net/udp file
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
    char local_address_string[INET_ADDRSTRLEN];
    char remote_address_string[INET_ADDRSTRLEN];
    uint32_t local_address_hex;
    uint32_t remote_address_hex;

    int ret_sscanf;

    proc_ipv4_udp_stat_file = fopen(proc_ipv4_udp_stat_filename, "r");
    if (proc_ipv4_udp_stat_file == NULL) {
        fprintf(stderr, "ERROR: failed to open IPv4 UDP stats file %s: %s\n", proc_ipv4_udp_stat_filename, strerror(errno));
        return;
    }

    while (fgets(line, sizeof(line), proc_ipv4_udp_stat_file) != NULL) {
        /* read fields from each stat line */
        ret_sscanf = sscanf(line, format, &index, local_address, &local_port, remote_address, &remote_port, &socket_state, &tx_queue, &rx_queue, &timer_active, &time_length, &retry, &uid, &timeout, &socket_inode);

        if (ret_sscanf < 11) {
            continue;
        }

        if (input_socket_inode == socket_inode) {
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

            /* print network connection stats */
            fprintf(stdout, "UDP  %-13s%s:%d  %s:%d  %ld  %ld\n", tcp_state[socket_state], local_address_string, local_port, remote_address_string, remote_port, tx_queue, rx_queue);
            fflush(stdout);
        }
    }

    fclose(proc_ipv4_udp_stat_file);
}
