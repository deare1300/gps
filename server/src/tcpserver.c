#include "include/tcpserver.h"
#include "include/cmptime.h"
#include "include/gpsact.h"
#include "include/analysis.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct thread_arg
{
	int conn_fd;
	struct share_msg *msg;
};

static void *socket_process(void *conn)
{
	char rcv[RCV_MAX_LEN];
	int socket_conn;
	struct share_msg *msg;
	struct thread_arg *targ = (struct thread_arg *) conn;
	socket_conn = targ->conn_fd;
	msg = targ->msg;
	int n;
	void *ret = (void *) 0;
    printf("socket_process\n");
	while(1)
	{
		if((n = recv(socket_conn, rcv, RCV_MAX_LEN, 0)) < 0)
		{
			ret = (void *) -1;
			break;
		}
		else if(n == 0)
		{
			break;
		}
		else
		{
			rcv[n] = 0;
            printf("size is %d\n", n);
            printf("server accept socket=%d recv_msg %s\n", socket_conn, rcv);
			if(strcmp(rcv, GPS_START_CMD) == 0)
			{
				if(start_gps_action(targ->msg->fds) == 0)
				{
					send(socket_conn, START_GPS_SUCCESS, strlen(START_GPS_SUCCESS), 0);
				}
				else
				{
					 send(socket_conn, START_GPS_FAILED, strlen(START_GPS_FAILED), 0);
				}
			}
			else if(strcmp(rcv, GPS_CLOSE_CMD) == 0)
			{
				if(stop_gps_action(targ->msg->fds) == 0)
				{
					send(socket_conn, STOP_GPS_SUCCESS, strlen(STOP_GPS_SUCCESS), 0);
				}
				else
				{
					 send(socket_conn, STOP_GPS_FAILED, strlen(STOP_GPS_FAILED), 0);
				}
				
			}
			else if(n == sizeof(TIME_STRUCT))
			{
				TIME_STRUCT res = {0, 0};
				int r = analysis(rcv, msg->time, &res);
				if(res.tv_sec == 0)
				{
					send(socket_conn, GPS_ERROR_MESSAGE, strlen(GPS_ERROR_MESSAGE), 0);
				}
				else
				{
					send(socket_conn, &res, sizeof(TIME_STRUCT), 0);
				}
			}
			else
			{
				send(socket_conn, GPS_UNKNOWN_CMD, strlen(GPS_UNKNOWN_CMD), 0);
			}
		}
	}
	free(targ);
    close(socket_conn);
    printf("socket = %d closed\n", socket_conn);
	return ret;
}

void *start_server(void *sfd)
{
	struct sockaddr_in A, B;
	int server_socket, conn_socket;
	socklen_t len  = sizeof(A);
	struct share_msg *msg = (struct share_msg *) sfd;
	server_socket = socket(AF_INET,SOCK_STREAM,0);

	if(server_socket < 0)
	{
		perror("server socket init failed \n");
		//return ((void *) -1);
        exit(-1);
	}
	A.sin_family = AF_INET;
	A.sin_port = htons(SERVER_PORT);
	A.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(server_socket ,(struct sockaddr *)&A,sizeof(A)) < 0)
	{
		perror("bind server failed\n");
		//return ((void *) -2);
        exit(-2);
	}
	if(listen(server_socket,1024) < 0)
	{
		perror("listen server failed\n");
		//return ((void *) -3);
        exit(-3);
	}
	while(1)
	{
		if((conn_socket = accept(server_socket,(struct sockaddr *)&B ,&len)) > 0)
		{
			int ret_td;
			struct thread_arg *arg = malloc(sizeof(struct thread_arg));
			pthread_t tid;
			if(arg == NULL)
			{
				printf("memory overflow in malloc start server\n");
				return (void *) -1;
			}
			arg->conn_fd = conn_socket;
			arg->msg = msg;
            printf("connect the socket= %d\n", conn_socket);
			if((ret_td = pthread_create(&tid, NULL, socket_process, (void *)arg)) < 0)
			{
				perror("create socket thread\n");
				close(conn_socket);
			}
            else
            {
                printf("create pthread success\n");
            }
		
		}
		else
		{
			perror("socket connection failed\n");
		}
		
	}
	return ((void *) 0);

}
