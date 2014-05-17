#ifndef GPSMSG_H_
#define GPSMSG_H

#define GPS_PACKAGE_ERROR -1  // PACKAGE ERROR
#define GPS_NOT_READY -2  // GPS not ready V
#define GPS_READY 0   //GPS ready
#define GPS_CLOSE_CMD "GPS CLOSE"
#define GPS_START_CMD "GPS START"
#define GPS_ERROR_MESSAGE "GPS ERROR"
#define GPS_UNKNOWN_CMD  "UNKNOWN CMD"
#define START_GPS_SUCCESS "GPS STARTED SUCCESS"
#define STOP_GPS_SUCCESS "GPS STOPPED SUCCESS"
#define START_GPS_FAILED "GPS STARTED FAILED"
#define STOP_GPS_FAILED "GPS STOPPED FAILED"
#define GPS_FILE "/dev/ttyS1"
#define PPS_FILE "/root/test/dcdtime"
#define GPS_FD_INDEX 0
#define PPS_FD_INDEX 1
#define GPS_TIME_OVERFLOW -400
#define MSG_LEN 100

#define TIME_MINUS 8*3600
#endif
