#include <stdio.h>
#include <stdlib.h>
#include "udp_communication.h"

const int BUFFER_SIZE = 100;

int main(int argc, char *argv[]) {
    printf("lol, server program starts~\n");
    int sd = open_udp_socket(20000, NULL);
    if (sd == -1) {
        exit(-1);
    }

    while (1) {
        printf("waitting.....\n");
        struct sockaddr_in addr;
        char message[BUFFER_SIZE * sizeof(char)];
        int rc = receive_msg(sd, &addr, message, BUFFER_SIZE);
        if (rc > 0) {
            printf("%s", message);
            char *reply = "haha\n";
            rc = send_msg(sd, &addr, reply, sizeof(reply));
        }
    }
}