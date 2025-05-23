#include "../inc/ctime.h"

void save_time(struct timespec* t) {
    clock_gettime(CLOCK_REALTIME, t);
}

struct tm format_time(struct timespec t) {
    struct tm time;
    localtime_r(&t.tv_sec, &time);
    return time;
}