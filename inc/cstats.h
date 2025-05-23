#ifndef CSTATS_H_GUARD
#define CSTATS_H_GUARD

#include "csettings.h" // para struct info_container

// writes current stats to the file indicated in info->statistics_filename
void write_statistics(struct info_container* info);

#endif