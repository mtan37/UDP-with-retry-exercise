#include <arpa/inet.h>

int open_udp_socket(int port, char *host_name, int bind_to_port);
int send_msg(int sd, struct sockaddr_in *addr, void *buffer, size_t msg_len, int *retry_count);
int receive_msg(int sd, struct sockaddr_in *addr, void *buffer, size_t buffer_len, int drop_message);