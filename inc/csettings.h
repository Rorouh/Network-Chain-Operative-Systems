#ifndef CSETTINGS_H_GUARD
#define CSETTINGS_H_GUARD

#define MAX_WALLETS 100
#define MAX_SERVERS 100

#include "main.h"

struct info_container {
    float init_balance;
    int n_wallets;
    int n_servers;
    int buffers_size;
    int max_txs;
    char log_filename[256];
    char statistics_filename[256];
    int period;

    // Campos para estatísticas e PIDs
    int main_pid;
    int wallets_pids[MAX_WALLETS];
    int servers_pids[MAX_SERVERS];
    int main_received;
    int wallets_signed[MAX_WALLETS];
    int servers_processed[MAX_SERVERS];
    int main_receipts;
    float balances[MAX_WALLETS];

    // Outros campos que precises...
    int* terminate; // se usares flag de terminação
};

void read_args(char* filename, struct info_container* info);


void read_settings(char* filename, struct info_container* info);


#endif