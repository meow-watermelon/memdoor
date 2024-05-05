#ifndef NETWORK_H
#define NETWORK_H

struct netstat {
    char protocol[5];
    int socket_state;
    long int socket_inode;
    char local_address[128];
    int local_port;
    char remote_address[128];
    int remote_port;
    long int tx_queue;
    long int rx_queue;
    struct netstat *next_ptr;
};

extern struct netstat *load_netstat(char *protocol);
extern void free_netstat(struct netstat *input_netstat);
extern void get_connection_stats(long int input_socket_inode, struct netstat *input_netstat);

#endif /* NETWORK_H */
