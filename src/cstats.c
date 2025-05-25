// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

// src/cstats.c
#include <stdio.h>
#include <string.h>
#include "../inc/cstats.h"

/*
  To avoid adding new fields to info_container, we count:
   - main_received  = number of lines "trx " in log_filename
   - main_receipts  = number of lines "rcp " in log_filename
*/

void write_statistics(struct info_container* info, char*sf, int ntrx, int nrcp) {
    int main_received = ntrx, main_receipts = nrcp;

    FILE *fp = fopen(sf, "w");
    if (!fp) {
        perror("Error opening statistics file");
        return;
    }

    fprintf(fp, "Process Statistics:\n");
    fprintf(fp, "Main has PID [%d]\n", getpid());

    // Wallets
    fprintf(fp, "There were %d Wallets, PIDs [", info->n_wallets);
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "%d", info->wallets_pids[i]);
        if (i+1 < info->n_wallets) fputs(",", fp);
    }
    fputs("]\n", fp);

    // Servers
    fprintf(fp, "There were %d Servers, PIDs [", info->n_servers);
    for (int i = 0; i < info->n_servers; i++) {
        fprintf(fp, "%d", info->servers_pids[i]);
        if (i+1 < info->n_servers) fputs(",", fp);
    }
    fputs("]\n", fp);

    // Main received and receipts
    fprintf(fp, "Main received %d transaction(s)!\n", main_received);
    fprintf(fp, "Main read %d receipts.\n", main_receipts);

    // Wallets signed
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "Wallet #%d signed %d transaction(s)!\n",
                i, info->wallets_stats[i]);
    }

    // Servers processed
    for (int i = 0; i < info->n_servers; i++) {
        fprintf(fp, "Server #%d processed %d transaction(s)!\n",
                i, info->servers_stats[i]);
    }

    // Final balances
    fputs("Final Balances [", fp);
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "%.2f", info->balances[i]);
        if (i+1 < info->n_wallets) fputs(",", fp);
    }
    fputs("] SOT\n", fp);

    fclose(fp);
}
