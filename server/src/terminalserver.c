#include "include/tcpserver.h"
#include "include/cmptime.h"
#include "include/gpsact.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>


void init(struct share_msg *msg)
{
    printf("int init\n");
    msg->time->len = 0;
    //msg->time->lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&msg->time->lock, NULL);
    msg->fds->len = 0;
    pthread_mutex_init(&msg->fds->lock, NULL);
}

int main()
{
    struct cmp_time tm;
    struct setfds sfds;
    struct share_msg msg;
    int ret;
    pthread_t stid;
    pthread_t gtid;
    msg.time = &tm;
    msg.fds = &sfds;
    init(&msg);
    printf("init over\n");
    ret = pthread_create(&stid, NULL, start_server, (void *) &msg);
    if(ret < 0)
    {
        perror("create server socket failed\n");
        return -1;
    }
    ret = pthread_create(&gtid, NULL, run_gps, (void *) &msg);
    if(ret < 0)
    {
        perror("run gps error\n");
        return -1;
    }
    pthread_join(stid, NULL);
    pthread_join(gtid, NULL);
    //start_server((void*) &msg);    
    return 0;
}

