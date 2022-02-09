#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "udp_communication.h"

const char* ACK_CONTENT = "I am back~";
const size_t ACK_LENGTH = sizeof(ACK_CONTENT);
const unsigned int ACK_TIMEOUT = 5;// in second

int open_udp_socket(int port, char *toaddr) {
    printf("open udp socket to listen for requets at port %d\n", port);
    int sd;
    in_addr_t myipaddr;
    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(myaddr));

    if ((sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        return -1;

    myaddr.sin_family = AF_INET;
    myaddr.sin_port=htons(port);
    if (toaddr == NULL || (myipaddr = inet_addr(toaddr)) == -1) {
        myaddr.sin_addr.s_addr = INADDR_ANY;
    } else {
        myaddr.sin_addr.s_addr = myipaddr;
    }

    if(bind(sd , (struct sockaddr*)&myaddr, sizeof(myaddr) ) == -1) {
        close(sd);
        return -1;
    }
    printf("udp socket open successful\n");
    return sd;
}

int send_ack(int sd, struct sockaddr_in *dest_addr) {
    int addr_len = sizeof(struct sockaddr_in);
    printf("ack sent\n");
    return sendto(sd, ACK_CONTENT, ACK_LENGTH, 0, (struct sockaddr *) dest_addr, addr_len);
}

/**
 * @brief Waitting for an acknowledgement packet from the other side. 
 * Note this assumes that this function is used execlusive by the client, and 
 * the server will never send the client anything except ack....
 * 
 * @param sd 
 * @param expected_src_addr 
 * @param timeout 
 * @return int 
 */
int receive_ack(int sd, struct sockaddr_in *expected_src_addr, struct timeval timeout) {
    struct sockaddr_in src_addr;
    int addr_len = sizeof(struct sockaddr_in);
    char buffer[ACK_LENGTH];
   
    timeout.tv_sec = ACK_TIMEOUT;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    // should could add something to check the content of the ack TODO
    recvfrom(sd, buffer, ACK_LENGTH, 0, (struct sockaddr *) &src_addr, (socklen_t *) &addr_len);
    timeout.tv_sec = 0;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    printf("ack received\n");
    return 1;
}

/**
 * @brief Modified from Andrea and Ramzi's text book OPERATING
SYSTEMS, Figure 48.2... 
 */
int send_msg(int sd, struct sockaddr_in *dest_addr, char *buffer, int msg_len, int*retry_count) {
    int addr_len = sizeof(struct sockaddr_in);
    if (0 > sendto(sd, buffer, msg_len, 0, (struct sockaddr *) dest_addr, addr_len)) {
        exit(1);
    }

    (*retry_count) = 0;
    int receive_status;
    // set the timeout structure
    struct timeval timeout;
    timeout.tv_sec = ACK_TIMEOUT;
    timeout.tv_usec = 0;  
    while (-1 == (receive_status = receive_ack(sd, dest_addr, timeout))) {
        // retry send_ack for five times before timeout
        (*retry_count)++;
        if (5 <= *retry_count) return -1;
    }
    
    return receive_status;
}

int receive_msg(int sd, struct sockaddr_in *src_addr, char *buffer, int buffer_len) {
    int addr_len = sizeof(struct sockaddr_in);
    if (0 > recvfrom(
        sd, buffer, buffer_len,
        0, (struct sockaddr *) src_addr, (socklen_t *) &addr_len)) {
            exit(1);
    }
    return send_ack(sd, src_addr);
}