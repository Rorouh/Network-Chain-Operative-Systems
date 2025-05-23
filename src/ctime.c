#include "../inc/ctime.h"
#include <bits/time.h> // Bruno: CLOCK_REALTIME was giving me an error without this include
#include <linux/time.h> // Bruno: CLOCK_REALTIME was giving me an error without this include

void save_time(struct timespec* t) {
    clock_gettime(CLOCK_REALTIME, t);
}

struct tm format_time(struct timespec t) {
    struct tm time;
    localtime_r(&t.tv_sec, &time);
    return time;
}