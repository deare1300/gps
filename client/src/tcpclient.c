#include "include/tcpclient.h"
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static int get_client_socket();

static int get_client_socket()
{
    int sockfd;
    struct sockaddr_in dest_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("CLINET SOCKET INIT Error*****\n");
        return -1;
    }
    dest_addr.sin_family = AF_INET; /* host byte order */
    dest_addr.sin_port = htons(SERVER_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    bzero(&(dest_addr.sin_zero), 8);
    if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Connection Error***\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int run_client()
{
    int sockfd = get_client_socket();
    char sd[RCV_MAX_LEN];
    char rcv[RCV_MAX_LEN];
    ssize_t n;
    if(sockfd < 0)
    {
        perror("get_client_socket failed\n");
        return sockfd;
    }
    while(1)
    {
        if((n = read(STDIN_FILENO, sd, RCV_MAX_LEN)) < 0)
        {
            perror("READ ERROR\n");
            break;
        }
        else if(n == 0)
        {
            printf("input over\n");
            break;
        }
        else
        {
            send(sockfd, sd, n, 0);
            n = recv(sockfd, rcv, RCV_MAX_LEN, 0);
            if(n < 0)
            {
                perror("recv eroor\n");
                break;
            }
            else if(n == 0)
            {
                break;
            }
            else
            {
                rcv[n] = 0;
                printf("recv %s\n", rcv);
            }
        }
    }
    close(sockfd);
    return n;
}
