#ifndef CSETTINGS_H_GUARD
#define CSETTINGS_H_GUARD

#define MAX_WALLETS 100
#define MAX_SERVERS 100

#include "main.h"

void read_args(char* filename, struct info_container* info);


void read_settings(char* filename, struct info_container* info);


#endif