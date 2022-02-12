#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "udp_communication.h"

const int BUFFER_SIZE = 64 * 1024; // Make the buffer size 64k Byte

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: ./server <port> <message drop percentage> -d\n");
        exit(-1);
    }

    char *ptr;
    int server_port = strtol(argv[1], &ptr, 10);
    int drop_percent = strtol(argv[2], &ptr, 10) % 100;
    printf("lol, server program starts~\n");
    int sd = open_udp_socket(server_port, NULL, 1);
    if (sd == -1) {
        exit(-1);
    }

    int print_debug_message = 0;
    // check if debug flag is on
    if (argc > 3) {
        if (strcmp(argv[3], "-d") == 0) {
            print_debug_message = 1;
        }
    }

    srand(time(0));
    int r, drop_message;
    while (1) {
        struct sockaddr_in addr;
        char message[BUFFER_SIZE];

        r = rand() % 100;
        drop_message = 0;

        if (r < drop_percent) {
            drop_message = 1;
        }

        receive_msg(sd, &addr, message, BUFFER_SIZE, drop_message);

        if (print_debug_message == 1) {
            if (drop_message == 1) {
                printf("Dropped message %s\n", message);
            } else {
                printf("Sent ack for message %s\n", message);
            }
        }
    }
}
