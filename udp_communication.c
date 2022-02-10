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
const unsigned int ACK_TIMEOUT = 1;// in second

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
    printf("ack sent\n");

    int addr_len = sizeof(struct sockaddr_in);
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
    int return_status = 1;
   
    timeout.tv_sec = ACK_TIMEOUT;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    // should could add something to check the content of the ack TODO
    if (0 > recvfrom(sd, buffer, ACK_LENGTH, 0, (struct sockaddr *) &src_addr, (socklen_t *) &addr_len)) {
        return_status = -1;
        printf("receive ack timeout\n");
    }
    timeout.tv_sec = 0;
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    return return_status;
}

/**
 * @brief Modified from Andrea and Ramzi's text book OPERATING
SYSTEMS, Figure 48.2... 
 */
int send_msg(int sd, struct sockaddr_in *dest_addr, char *buffer, int msg_len, int*retry_count) {
    int addr_len = sizeof(struct sockaddr_in);

    (*retry_count) = 0;
    // set the timeout structure
    struct timeval timeout;
    timeout.tv_sec = ACK_TIMEOUT;
    timeout.tv_usec = 0;
    int received_ack = -1;

    while (received_ack == -1) {
        if (0 > sendto(sd, buffer, msg_len, 0, (struct sockaddr *) dest_addr, addr_len)) {
            printf("message sent failed with error %d\n", errno);
            exit(1);
        }

        if (-1 == receive_ack(sd, dest_addr, timeout)) {
            (*retry_count)++;
            printf("message is droppped... with retry count %d\n", *retry_count);
            if (5 <= *retry_count) return -1;
        } else {
            received_ack = 1;
        }
    }
    
    return received_ack;
}

int receive_msg(int sd, struct sockaddr_in *src_addr, char *buffer, int buffer_len, int drop_message) {
    int addr_len = sizeof(struct sockaddr_in);
    if (0 > recvfrom(
        sd, buffer, buffer_len,
        0, (struct sockaddr *) src_addr, (socklen_t *) &addr_len)) {
            return -1;
    }
    if (drop_message == 0) return send_ack(sd, src_addr);

    printf("dropping the messsage\n");
    return -1;
}