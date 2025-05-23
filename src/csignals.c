#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include "../inc/csignals.h"

void setup_signals(void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);   // Ctrl+C
    sigaction(SIGTERM, &sa, NULL);  // kill
    sigaction(SIGALRM, &sa, NULL);  // alarm
}

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nRecieved Ctrl-C (%d). Shutting down...\n", sig);
        // end_execution(...) !?
    } else if (sig == SIGALRM) {
        printf("Recieved SIGALRM (timer).\n");
        // passar aos "filhos" ?
    }
}