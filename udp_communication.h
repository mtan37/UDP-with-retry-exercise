#include <arpa/inet.h>

int open_udp_socket(int port, char *host_name);
int send_msg(int sd, struct sockaddr_in *addr, char *buffer, int n);
int receive_msg(int sd, struct sockaddr_in *addr, char *buffer, int n);