// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

// src/cstats.c
#include <stdio.h>
#include <string.h>
#include "../inc/cstats.h"

/*
  Para no añadir campos nuevos a info_container, contamos:
   - main_received  = nº de líneas "trx " en log_filename
   - main_receipts  = nº de líneas "rcp " en log_filename
*/


void write_statistics(struct info_container* info, char*sf, int ntrx, int nrcp) {
    int main_received = ntrx, main_receipts = nrcp;

    FILE *fp = fopen(sf, "w");
    if (!fp) {
        perror("Error abriendo fichero de estadísticas");
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

    // Main recibidos y recibos
    fprintf(fp, "Main received %d transaction(s)!\n", main_received);
    fprintf(fp, "Main read %d receipts.\n", main_receipts);

    // Wallets firmadas
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "Wallet #%d signed %d transaction(s)!\n",
                i, info->wallets_stats[i]);
    }

    // Servers procesadas
    for (int i = 0; i < info->n_servers; i++) {
        fprintf(fp, "Server #%d processed %d transaction(s)!\n",
                i, info->servers_stats[i]);
    }

    // Saldos finales
    fputs("Final Balances [", fp);
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "%.2f", info->balances[i]);
        if (i+1 < info->n_wallets) fputs(",", fp);
    }
    fputs("] SOT\n", fp);

    fclose(fp);
}
