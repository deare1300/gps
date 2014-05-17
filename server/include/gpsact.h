#ifndef GPSACT_H_
#define GPSACT_H_

struct cmp_time;
struct setfds;
struct share_msg;

void *run_gps(void *msg);
int start_gps_action(struct setfds* sfds);
int stop_gps_action(struct setfds* sfds);

#endif
