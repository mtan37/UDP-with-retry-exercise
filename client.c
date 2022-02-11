#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <unistd.h>
#include "udp_communication.h"

const size_t BUFFER_SIZE = 32 * 1024; // Make the buffer size 64k Byte
const size_t MAX_PACKET_SIZE = 16 * 1024;
const unsigned int ACK_TIMEOUT = 1;// in second
const unsigned S_TO_NS = 1000000000;

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("usage: ./client <server hostname> <port> <packet count> -R\n");
        exit(-1);
    }

    char *host = argv[1]; // take the host name of the server
    // take the port number of the server that is listening 
    char *ptr;
    int remote_port = strtol(argv[2], &ptr, 10);
    int packet_count = strtol(argv[3], &ptr, 10);

    int test_reliability_flag = 0;
    if (argc > 4 && strcmp(argv[4], "-R") == 0) {
        test_reliability_flag = 1;
    }

    printf("lol, client program starts~\n");
    // open a port to receive message 
    // only receive message from the server 
    int sd = open_udp_socket(-1, NULL, 0);
    if (sd == -1) {
        exit(-1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port=htons(remote_port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    // send the message
    char message[BUFFER_SIZE];
    int message_size = 0;

    if (test_reliability_flag == 0) {
        message_size = MAX_PACKET_SIZE;
        // send max size packet
        // memset(message, '0', message_size);
        message[message_size - 1] = '\0';
    } else {
        message_size = 2;
        // send max size packet
        memset(message, '0', message_size);
        message[message_size - 1] = '\0';
    }

    struct timespec begin_time, end_time;

    // set the timeout structure
    struct timeval timeout;
    timeout.tv_sec = ACK_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    int i, retry_count, packet_sent = 0;
    unsigned long sec_passed = 0, nsec_passed = 0;

    for (i = 0; i < packet_count; i++) {

        retry_count = 0;
        clock_gettime(CLOCK_REALTIME, &begin_time);
        if (-1 == send_msg(sd, &server_addr, message, message_size, &retry_count)) {
            printf("send message failed\n");
            exit(-1);
        }
        clock_gettime(CLOCK_REALTIME, &end_time);
        printf("message sent, with a re-try count of %d\n", retry_count);
        if (test_reliability_flag == 1) {
            printf("message content: %s\n", message);
            // append message content
            message_size += 1;
            // send max size packet
            message[message_size - 2] = '0';
            message[message_size - 1] = '\0';
        }
        // compute the time
        if (retry_count == 0){
            nsec_passed += end_time.tv_nsec - begin_time.tv_nsec;
            sec_passed += end_time.tv_sec - begin_time.tv_sec;
            packet_sent += 1;

            if (nsec_passed > S_TO_NS) {
                sec_passed += 1;
                nsec_passed -= S_TO_NS;
            }
        }
    }

    close(sd);

    // compute latency
    if (packet_sent > 0){
        nsec_passed =  nsec_passed/ (packet_sent * 2);
        sec_passed =  sec_passed/ (packet_sent * 2);
        printf("*********Latency*********\n");
        printf("*********%ld s and %ld ns*********\n", sec_passed, nsec_passed);
        printf("*********Throughput*********\n");
        printf("*********%ld s and %ld ns*********\n", sec_passed, nsec_passed);
    } else {
        printf("No packets were sent\n");
    }
}