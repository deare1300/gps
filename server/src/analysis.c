#include "include/analysis.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#define NOT_IN -1
#define IN_LOST 1
#define IN_FIND 0
#define POINT_NUM 60


#define RELATION 1000000000

static void parse_time(TIME_STRUCT *left, TIME_STRUCT *right, struct cmp_time *cmp, int index, TIME_STRUCT *res);



/**
 line y = a1 * x + a0;
**/

struct line
{
	double a0;
	double a1;
};

static int find_target(struct cmp_time *cmp, int len, client_t *t, int *index)
{
	int i = 0;
	if(len <= 0 || cmp->pps[0].tv_sec > t->tv_sec || cmp->pps[len-1].tv_sec < t->tv_sec)
	{
		return NOT_IN;
	}
	
	for(i = 0; i < len; ++i)
	{
		if(cmp->pps[i].tv_sec > t->tv_sec)
		{
			break;
		}
	}
	if(index != NULL)
	{
		*index = i-1;
	}
	if(i == len)
	{
		return IN_LOST;
	}
	return IN_FIND;
}


//square line algorithm

static void build_line(struct cmp_time *cmp, int start, int end, struct line *line)
{
	double xy = 0.0, xsum = 0.0, ysum = 0.0, x_square_sum = 0.0, x_ave, y_ave;
	int i;
	int total = end - start + 1;
	long long x, y;
	for(i = start; i < end; ++i)
	{
		x = (cmp->pps[i].tv_sec - cmp->pps[start].tv_sec) * RELATION + cmp->pps[i].tv_nsec - cmp->pps[start].tv_nsec;
		y = (cmp->gps[i] - cmp->pps[i].tv_sec)*RELATION - cmp->pps[i].tv_nsec;
		xy += x*y;
		xsum += x;
		ysum += y;
		x_square_sum += x*x;
	}
	x_ave = xsum / total;
	y_ave = ysum / total;
	line->a1 = (xy - total*x_ave*y_ave)/(x_square_sum - total*x_ave*x_ave);
	line->a0 = y_ave - line->a1*x_ave;
}

static TIME_STRUCT convert_mytime(client_t *t)
{
    TIME_STRUCT temp;
    temp.tv_sec  = t->tv_sec;
    temp.tv_nsec = t->tv_usec * 1000;
    return temp;
}

static long get_y(struct line *line, double x)
{
	return (long)(line->a1*x + line->a0);
}

static void get_client_time(struct cmp_time *cmp, int len, int tg_index, client_t *target, TIME_STRUCT *res)
{
	struct line left_line, right_line;
	int start, end;
	TIME_STRUCT left_time, right_time;
    TIME_STRUCT tm_target = convert_mytime(target);
	long y;
	memset(&left_line, 0, sizeof(struct line));
	memset(&right_line, 0, sizeof(struct line));
	start = (tg_index > POINT_NUM) ? (tg_index - POINT_NUM) : 1;
	end = tg_index;
	build_line(cmp, start, end, &left_line);
    y = get_y(&left_line, (tm_target.tv_sec - cmp->pps[start].tv_sec) * RELATION + tm_target.tv_nsec - cmp->pps[start].tv_nsec);
    left_time.tv_sec = cmp->pps[start].tv_sec + y / RELATION;
    left_time.tv_nsec = cmp->pps[start].tv_nsec + y % RELATION;
	start = tg_index;
	end = (tg_index+POINT_NUM) < len ? (tg_index+POINT_NUM): (len-1);
	build_line (cmp, start, end, &right_line);
    y = get_y(&right_line, (tm_target.tv_sec - cmp->pps[start].tv_sec) * RELATION + tm_target.tv_nsec - cmp->pps[start].tv_nsec);
    right_time.tv_nsec = cmp->pps[start].tv_sec + y / RELATION;
    right_time.tv_nsec = cmp->pps[start].tv_nsec + y % RELATION;
    parse_time(&left_time, &right_time, cmp, tg_index, res);
}

static void parse_time(TIME_STRUCT *left, TIME_STRUCT *right, struct cmp_time *cmp, int index, TIME_STRUCT *res)
{
    res->tv_sec = (left->tv_sec + right->tv_sec) / 2;
    res->tv_nsec = (left->tv_nsec + right->tv_nsec) / 2;
}
int analysis(char *src, struct cmp_time *cmp, TIME_STRUCT *res)
{
	client_t *target = (client_t*) src;
	int target_index = 0;
	int cur_len = 0;
	int ret;
	if(res != 0)
		memset(res, 0, sizeof(TIME_STRUCT));
	if(src == 0 || cmp == 0)
	{
		return -1;
	}
	pthread_mutex_lock(&cmp->lock);
	cur_len = cmp->len;
	pthread_mutex_unlock(&cmp->lock);
	ret = find_target(cmp, cur_len, target, &target_index);
	if(ret == NOT_IN)
	{
		return -1;
	}
	get_client_time(cmp, cur_len, target_index, target, res);

	return 0;
}
