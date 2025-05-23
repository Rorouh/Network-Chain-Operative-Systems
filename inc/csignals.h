#ifndef CSIGNALS_H_GUARD
#define CSIGNALS_H_GUARD

void setup_signals(void (*handler)(int));
void signal_handler(int sig);

#endif