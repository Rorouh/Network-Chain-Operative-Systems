// src/cstats.c
#include <stdio.h>
#include <string.h>
#include "../inc/cstats.h"

/*
  Para no añadir campos nuevos a info_container, contamos:
   - main_received  = nº de líneas "trx " en log_filename
   - main_receipts  = nº de líneas "rcp " en log_filename
*/

static void count_log_ops(const char *logfile, int *n_trx, int *n_rcp) {
    FILE *f = fopen(logfile, "r");
    if (!f) return;
    char buf[256];
    *n_trx = *n_rcp = 0;
    while (fgets(buf, sizeof(buf), f)) {
        // Después del timestamp, buf contiene " trx ", " rcp ", etc.
        if (strstr(buf, " trx ") == buf+18) (*n_trx)++;
        if (strstr(buf, " rcp ") == buf+18) (*n_rcp)++;
    }
    fclose(f);
}

void write_statistics(struct info_container* info) {
    int main_received = 0, main_receipts = 0;
    count_log_ops(info->log_filename, &main_received, &main_receipts);

    FILE *fp = fopen(info->statistics_filename, "w");
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
