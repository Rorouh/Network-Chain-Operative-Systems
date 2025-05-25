// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#ifndef CSETTINGS_H_GUARD
#define CSETTINGS_H_GUARD

#define MAX_WALLETS 100
#define MAX_SERVERS 100

#include "main.h"

void read_args(char* filename, struct info_container* info);

void read_settings(char *filename, char *lf, char *sf, int *p);
#endif