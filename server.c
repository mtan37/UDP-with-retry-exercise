#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "udp_communication.h"

const int BUFFER_SIZE = 65507; // MAX SIZE

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: ./server <message drop percentage>\n");
        exit(-1);
    }

    char *ptr;
    int drop_percent = strtol(argv[1], &ptr, 10) % 100;
    printf("lol, server program starts~\n");
    int sd = open_udp_socket(9999, NULL);
    if (sd == -1) {
        exit(-1);
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
    }
}