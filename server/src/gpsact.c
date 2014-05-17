#include "include/gpsact.h"
#include "include/cmptime.h"
#include "include/gpsmsg.h"
#include "include/fileop.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
/*
void *run_gps(struct cmp_time *times, struct setfds *sfds);
int start_gps_action(struct setfds *sfds);
int stop_gps_action(struct setfds *sfds);
*/

static void modify(TIME_STRUCT *pps_time);
static int getTime(char *package, LONG *tm);
static int parse_gps_package(char *package, LONG *tm);

int init_fds(struct setfds *sfds)
{
    int fd;
    int ret = 0;
	pthread_mutex_lock(&sfds->lock);
    fd = init_open_file(GPS_FILE);
    if(fd < 0)
    {
        ret = -1;
        goto failed;
    }
    sfds->fds[GPS_FD_INDEX] = fd;
    fd = init_open_file(PPS_FILE);
    if(fd < 0)
    {
        close(sfds->fds[GPS_FD_INDEX]);
        ret = -2;
        goto failed;
    }
    sfds->fds[PPS_FD_INDEX] = fd;
    sfds->len = 2;
    failed:
	pthread_mutex_unlock(&sfds->lock);
    return ret;
}

int start_gps_action(struct setfds *sfds)
{
	int ret = -1;
	pthread_mutex_lock(&(sfds->lock));
	if(sfds->fds[GPS_FD_INDEX] > 0 && sfds->fds[PPS_FD_INDEX] > 0)
	{
		ret = 0;
	}
	else if(init_fds(sfds) == 0)
	{
		ret = 0;
	}
	pthread_mutex_unlock(&(sfds->lock));
	return ret;
	
}

int stop_gps_action(struct setfds *sfds)
{
	pthread_mutex_lock(&(sfds->lock));
	close(sfds->fds[GPS_FD_INDEX]);
	close(sfds->fds[PPS_FD_INDEX]);
	sfds->fds[GPS_FD_INDEX] = -1;
	sfds->fds[PPS_FD_INDEX] = -1;
	pthread_mutex_unlock(&(sfds->lock));
	return 0;
}


static void modify(TIME_STRUCT *pps_time)
{
	if(pps_time->tv_nsec < 100000000)
	{
		--pps_time->tv_sec;
		pps_time->tv_nsec += 1000000000-100000000;
	}
	else
	{
		pps_time->tv_nsec -= 100000000;
	}
	pps_time->tv_sec -= TIME_MINUS;
}

static int getTime(char *package, LONG *tm)
{
        char buf[MSG_LEN];
        struct tm tm_time;
        int i;
        int index = 0;
        for(i = 0; i < 6; i++)
        {
                buf[index++] = package[53+i];
        }

        for(i = 0; i < 6; i++)
        {
                buf[index++] = package[7+i];
        }
        buf[12] = 0;
        strptime(buf, "%d%m%y%H%M%S", &tm_time);
        *tm = mktime(&tm_time);
        return 0;
}

static int parse_gps_package(char *package, LONG *tm)
{
        int plen = strlen(package);
        char *to = "$GPRMC";
        int i;
        if(package == NULL)
        {
                return GPS_PACKAGE_ERROR;
        }
        for(i = 0; i < 6; i++)
        {
                if(package[i] != to[i])
                {
                        return GPS_NOT_READY;
                }
        }

        if(plen < 14 || package[14] != 'A')
        {
                return GPS_NOT_READY;
        }
        getTime(package, tm);
	return GPS_READY;
}


void *run_gps(void *msg)
{
	int fd, maxfd = 0;
	int changed = 0;
	int total_gps, total_pps;

	TIME_STRUCT pps_time;
	LONG gps_time;
	int i;

	int n;
	int ret;
	char buf[MSG_LEN];
    struct cmp_time *times;
    struct setfds *sfds;
    struct share_msg *smsg = (struct share_msg*)msg;
    sfds = smsg->fds;
	fd_set rset;

	while(sfds->len == 0)
	{
		printf("nothing to do\n");
        sleep(1);
	}
    
	pthread_mutex_lock(&(sfds->lock));
	for(i = 0; i < sfds->len; ++i)
	{
	    if(maxfd < sfds->fds[i])
		{
	        maxfd = sfds->fds[i];
		}
	}
    pthread_mutex_unlock(&(sfds->lock));
    ++maxfd;
    
	while(1)
	{
		FD_ZERO(&rset);
		pthread_mutex_lock(&(sfds->lock));
		for(i = 0; i < sfds->len; ++i)
		{
			if(sfds->fds[i] > -1)
				FD_SET(sfds->fds[i], &rset);
			else
				break;
		}
		if(i != sfds->len);
		{
			pthread_mutex_unlock(&(sfds->lock));
			break;
		}
		pthread_mutex_unlock(&(sfds->lock));
		switch((fd=select(maxfd, &rset, NULL, NULL, NULL)))
		{
			case -1:
				return (void *)-1;
				break;
			case 0:
				break;
			default:
				pthread_mutex_lock(&(sfds->lock));
				if(FD_ISSET(sfds->fds[GPS_FD_INDEX], &rset))
				{
					n = read(sfds->fds[GPS_FD_INDEX], buf, MSG_LEN);
					buf[n] = 0;
					ret = parse_gps_package(buf, &gps_time);
					if(ret == GPS_READY)
					{
						long long minus = 0;
						++total_gps;
						pthread_mutex_lock(&times->lock);
						if(times->len > MAX_TIME_SIZE)
						{
							pthread_mutex_unlock(&times->lock);
							return ((void *) GPS_TIME_OVERFLOW);
						}
						times->gps[times->len]  = gps_time;
						memset(&times->pps[times->len], 0, sizeof(TIME_STRUCT));
						if(changed)
						{
							minus = (pps_time.tv_sec - gps_time)*1000000000 + pps_time.tv_nsec;
							times->pps[times->len] = pps_time;
						}
						changed = 0;
						strcpy(buf, times->package[times->len]);
						++times->len;
						printf("%ld\t%ld.%ld\t%lld\n",gps_time, pps_time.tv_sec,pps_time.tv_nsec, minus);
						pthread_mutex_unlock(&times->lock);
					}
					else if(ret == GPS_PACKAGE_ERROR)
					{
					}			
				}
				if(FD_ISSET(sfds->fds[PPS_FD_INDEX], &rset))
				{
					n = read(sfds->fds[PPS_FD_INDEX], &pps_time, sizeof(pps_time));
					buf[n] = 0;
					if(n != sizeof(TIME_STRUCT) || gps_time == 0)
					{
					}
					else
					{
						++total_pps;
						pthread_mutex_lock(&times->lock);
						modify(&pps_time);
						changed = 1;
						pthread_mutex_unlock(&times->lock);
					}
				}			
				pthread_mutex_unlock(&(sfds->lock));
				break;
		}
		sleep(50);
	}
	return ((void *) 0);
}
