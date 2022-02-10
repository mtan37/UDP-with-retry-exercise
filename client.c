#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "udp_communication.h"

const int BUFFER_SIZE = 65527; // MAX SIZE

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: ./client <server hostname> <port>\n");
        exit(-1);
    }

    char *host = argv[1]; // take the host name of the server
    // take the port number of the server that is listening 
    char *ptr;
    int remote_port = strtol(argv[2], &ptr, 10);

    printf("lol, client program starts~\n");
    // open a port to receive message 
    // only receive message from the server 
    int sd = open_udp_socket(20001, NULL);
    if (sd == -1) {
        exit(-1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port=htons(remote_port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    // send the message
    char message[BUFFER_SIZE * sizeof(char)];

    // fill in the message with character s
    memset(message, '0', (BUFFER_SIZE - 1)*sizeof(char));
    message[BUFFER_SIZE - 1] = '\0';

    struct timespec begin_time, end_time;
    int retry_count = 0;

    printf("about to send packet\n");
    clock_gettime(CLOCK_REALTIME, &begin_time);
    if (-1 == send_msg(sd, &server_addr, message, BUFFER_SIZE, &retry_count)) {
        printf("send message failed\n");
        exit(-1);
    }
     printf("send message failed\n");
    clock_gettime(CLOCK_REALTIME, &end_time);

    // compute the time
    printf("this is to check if the server will account for message waitting time... %ld\n", end_time.tv_sec - begin_time.tv_sec);
    printf("this is to check if the server will account for message waitting time... %ld\n", end_time.tv_nsec - begin_time.tv_nsec);
    printf("retry count is %d\n", retry_count);
}