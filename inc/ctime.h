// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#ifndef CTIME_H_GUARD
#define CTIME_H_GUARD

#include <time.h>
#include <stdlib.h>
#include <stdio.h>


struct timestamps {
    struct timespec main;
    struct timespec wallets;
    struct timespec servers;
};

void save_time(struct timespec* t);

struct tm format_time(struct timespec t);

#endif