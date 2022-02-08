#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "udp_communication.h"

const int BUFFER_SIZE = 100;

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
    memset(message, 'S', (BUFFER_SIZE - 1)*sizeof(char));
    message[BUFFER_SIZE - 1] = '\0';
    printf("the conent of the message to be sent is %s\n", message);
    int rc = send_msg(sd, &server_addr, message, BUFFER_SIZE);
    printf("send message status is %d\n", rc);
}