#ifndef CMPTIME_H_
#define CMPTIME_H_

#include "include/gpsmsg.h"
#include<time.h>
#include<pthread.h>
#define FD_MAX 10
#define MAX_TIME_SIZE 800

typedef struct timespec TIME_STRUCT;
typedef long LONG;

struct cmp_time
{
        int len;
        TIME_STRUCT pps[MAX_TIME_SIZE];
        LONG        gps[MAX_TIME_SIZE];
        char package[MAX_TIME_SIZE][MSG_LEN];
        pthread_mutex_t lock;
};


struct setfds
{
        int len;
        int fds[FD_MAX];
        pthread_mutex_t lock;
};


struct share_msg
{
        struct cmp_time *time;
        struct setfds   *fds;
};

#endif
