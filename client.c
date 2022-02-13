#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include "udp_communication.h"

const size_t BUFFER_SIZE = 64 * 1024; // Make the buffer size 64k Byte
const size_t MAX_PACKET_SIZE_K = 32;
const unsigned long KB_TO_B = 1024;
const size_t MAX_PACKET_SIZE = MAX_PACKET_SIZE_K * KB_TO_B;
const unsigned int ACK_TIMEOUT_MS = 500;// in second
const unsigned long S_TO_NS = 1000000000;
const unsigned long B_PER_MB = 1048576; // byte per megabyte

/**
 * @brief Code staright from https://stackoverflow.com/questions/53708076/what-is-the-proper-way-to-use-clock-gettime... 
 */
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += S_TO_NS;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= S_TO_NS;
        td->tv_sec++;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("usage: ./client <server hostname> <port> <packet size in k> <packet count> -R\n");
        exit(-1);
    }

    char *host = argv[1]; // take the host name of the server
    // take the port number of the server that is listening 
    char *ptr;
    int remote_port = strtol(argv[2], &ptr, 10);
    int packet_size_k = strtol(argv[3], &ptr, 10);
    int packet_count = strtol(argv[4], &ptr, 10);

    if (packet_size_k > MAX_PACKET_SIZE_K) {
        printf("packet size can't exceed 32k!!!!\n");
        exit(-1);
    }

    int test_reliability_flag = 0;
    if (argc > 5 && strcmp(argv[5], "-R") == 0) {
        printf("reliability flag is set. Packet size arg will be ignored\n");
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
    printf("%s\n", host);
    
    struct in_addr *in_addr;
    struct hostent *host_entry;
    if ((host_entry = gethostbyname(host)) == NULL)
        return -1;
    in_addr = (struct in_addr *) host_entry->h_addr;
    server_addr.sin_addr = *in_addr;

    // send the message
    char message[BUFFER_SIZE];
    int message_size = 0;

    if (test_reliability_flag == 0) {
        message_size = packet_size_k * KB_TO_B;
        // send max size packet
        // memset(message, '0', message_size);
        message[message_size - 1] = '\0';
    } else {
        message_size = 2;
        // send max size packet
        memset(message, '0', message_size);
        message[message_size - 1] = '\0';
    }

    struct timespec begin_time, end_time, elapsed;
    double round_trip = -1; // round trip in ns

    // set the timeout structure
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = ACK_TIMEOUT_MS;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    int i, retry_count;
    unsigned long long sec_passed = 0, nsec_passed = 0, packet_sent = 0;

    for (i = 0; i < packet_count; i++) {

        retry_count = 0;
        clock_gettime(CLOCK_MONOTONIC, &begin_time);
        if (-1 == send_msg(sd, &server_addr, message, message_size, &retry_count)) {
            printf("send message failed\n");
            exit(-1);
        }
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        if (test_reliability_flag == 1) {
            printf("message sent, with a re-try count of %d\n", retry_count);
            printf("message content: %s\n", message);
            // append message content
            message_size += 1;
            // send max size packet
            message[message_size - 2] = '0';
            message[message_size - 1] = '\0';
        }
        sub_timespec(begin_time, end_time, &elapsed);
        nsec_passed += elapsed.tv_nsec;
        sec_passed += elapsed.tv_sec;
        packet_sent += 1;

        if (nsec_passed > S_TO_NS) {
            sec_passed += 1;
            nsec_passed -= S_TO_NS;
        }

        // check round_trip, get the smallest value
        if (round_trip < 0 || (elapsed.tv_sec <= 0 && elapsed.tv_nsec < round_trip)) {
            round_trip = elapsed.tv_nsec;
        }
    }

    close(sd);

    if (packet_sent > 0){
        long double ns_in_s= nsec_passed / S_TO_NS;
        double throughput = packet_sent * message_size / (sec_passed + ns_in_s) / B_PER_MB;
        double avg_round_trip = (sec_passed * S_TO_NS + nsec_passed) / packet_sent;
        printf("*********Min Roundtrip*********\n");
        printf("*********%f ns*********\n", round_trip);
        printf("*********Avg Roundtrip*********\n");
        printf("*********%f ns*********\n", avg_round_trip);
        printf("*********Throughput*********\n");
        printf("*********%f MB/s*********\n", throughput);
    } else {
        printf("No packets were sent\n");
    }
}
