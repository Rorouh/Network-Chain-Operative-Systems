#include <stdio.h>
#include "../inc/csignals.h"
#include <signal.h>
#include <unistd.h>


static struct info_container* info;
static struct buffers* buffs;
int p = 0;
void setup_ctrlC_signal(struct info_container* info_main, struct buffers* buffs_main) {
    info = info_main;
    buffs = buffs_main;
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}
void setup_ctrlC_signal_parent(void) {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

void setup_alarm(int period) {
    p = period;
    signal(SIGALRM, signal_handler);
    alarm(p);
}

void signal_handler(int sig){
    if (sig == SIGINT) {
        printf("\nRecieved Ctrl-C. Shutting down...\n");
        end_execution(info, buffs);
    
    } else if (sig == SIGALRM) {
        print_alarm();
    }
}

void print_alarm(){
    struct timespec now;
    save_time(&now);
    printf("\n");
    for(int i = 0; i < info->buffers_size; i++) {
        if(buffs->buff_main_wallets->ptrs[i] != 0) {
            struct transaction tx = buffs->buff_main_wallets->buffer[i];
            double time = (now.tv_sec  - tx.change_time.main.tv_sec)
                            + (now.tv_nsec - tx.change_time.main.tv_nsec) * 1e-9;
            printf("%d %.3f\n", tx.id, time);            
        }

        if(buffs->buff_servers_main->ptrs[i] != 0){
            struct transaction tx = buffs->buff_servers_main->buffer[i];
            double time = (now.tv_sec  - tx.change_time.main.tv_sec)
                            + (now.tv_nsec - tx.change_time.main.tv_nsec) * 1e-9;
            printf("%d %.3f\n", tx.id, time);  
        }
    }

    int in = buffs->buff_wallets_servers->ptrs->in;
    int out = buffs->buff_wallets_servers->ptrs->out;
    for(int i = out; i != in; i = (i+1)%info->buffers_size){
        struct transaction tx = buffs->buff_wallets_servers->buffer[i];
        double time = (now.tv_sec  - tx.change_time.main.tv_sec)
                        + (now.tv_nsec - tx.change_time.main.tv_nsec) * 1e-9;
        printf("%d %.3f\n", tx.id, time);
    }

    alarm (p);  
}