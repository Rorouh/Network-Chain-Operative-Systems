#include <stdio.h>
#include "../inc/cstats.h"

void write_statistics(struct info_container* info) {
    FILE *fp = fopen(info->statistics_filename, "a");
    if (!fp) {
        perror("Erro ao abrir ficheiro de estatísticas");
        return;
    }

    fprintf(fp, "Process Statistics:\n");
    fprintf(fp, "Main has PID [%d]\n", info->main_pid);

    // Wallets
    fprintf(fp, "There were %d Wallets, PIDs [", info->n_wallets);
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "%d", info->wallets_pids[i]);
        if (i < info->n_wallets - 1) fprintf(fp, ",");
    }
    fprintf(fp, "]\n");

    // Servers
    fprintf(fp, "There were %d Servers, PIDs [", info->n_servers);
    for (int i = 0; i < info->n_servers; i++) {
        fprintf(fp, "%d", info->servers_pids[i]);
        if (i < info->n_servers - 1) fprintf(fp, ",");
    }
    fprintf(fp, "]\n");

    // Main transactions
    fprintf(fp, "Main received %d transaction(s)!\n", info->main_received);

    // Wallets signed
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "Wallet #%d signed %d transaction(s)!\n", i, info->wallets_signed[i]);
    }

    // Servers processed
    for (int i = 0; i < info->n_servers; i++) {
        fprintf(fp, "Server #%d processed %d transaction(s)!\n", i, info->servers_processed[i]);
    }

    // Main receipts
    fprintf(fp, "Main read %d receipts.\n", info->main_receipts);

    // Final balances
    fprintf(fp, "Final Balances [");
    for (int i = 0; i < info->n_wallets; i++) {
        fprintf(fp, "%.2f", info->balances[i]);
        if (i < info->n_wallets - 1) fprintf(fp, ",");
    }
    fprintf(fp, "] SOT\n");

    fclose(fp);
}