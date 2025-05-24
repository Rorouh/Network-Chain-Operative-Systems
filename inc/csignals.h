#ifndef CSIGNALS_H_GUARD
#define CSIGNALS_H_GUARD


#include <signal.h>
#include "main.h"

void setup_ctrlC_signal(struct info_container* info_main, struct buffers* buffs_main);
void setup_ctrlC_signal_parent(void);
void setup_alarm(void);
void signal_handler(int sig);
#endif