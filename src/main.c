// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../inc/memory.h"
#include "../inc/process.h"
#include "../inc/wallet.h"
#include "../inc/server.h"

/*
    Funcion para que los argumentos que entran en main() se inicializen con
    las variables que necestiamos para la blockchain
*/
void main_args(int argc, char *argv[], struct info_container *info) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s init_balance n_wallets n_servers buffers_size max_txs\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // transformacion de valores y las asignamos para la info
    info->init_balance = atof(argv[1]);
    info->n_wallets = atoi(argv[2]);
    info->n_servers = atoi(argv[3]);
    info->buffers_size = atoi(argv[4]);
    info->max_txs = atoi(argv[5]);

    //Comprobamos que los valores sean minimo mayores que 0 sino no los aceptamos
    if (info->init_balance < 0 || info->n_wallets <= 0 || info->n_servers <= 0 ||
        info->buffers_size <= 0 || info->max_txs <= 0) {
        fprintf(stderr, "Error: Argumentos inválidos.\n");
        exit(EXIT_FAILURE);
    }
}
/*
    Funcion para reservar la memoria de los struct que tenemos de cada fichero
*/
void create_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    info->wallets_pids = (int*) allocate_dynamic_memory(sizeof(int) * info->n_wallets);
    info->servers_pids = (int*) allocate_dynamic_memory(sizeof(int) * info->n_servers);

    buffs->buff_main_wallets = (struct ra_buffer*) allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_wallets_servers = (struct circ_buffer*) allocate_dynamic_memory(sizeof(struct circ_buffer));
    buffs->buff_servers_main = (struct ra_buffer*) allocate_dynamic_memory(sizeof(struct ra_buffer));

}
  
void create_shared_memory_structs(struct info_container* info, struct buffers* buffs) {
    // Buffer Main → Wallets (ra_buffer)
    buffs->buff_main_wallets->ptrs = (int*) create_shared_memory(ID_SHM_MAIN_WALLETS_PTR, sizeof(int) * info->buffers_size);
    buffs->buff_main_wallets->buffer = (struct transaction*) create_shared_memory(ID_SHM_MAIN_WALLETS_BUFFER, sizeof(struct transaction) * info->buffers_size);

    // Buffer Wallets → Servers (circ_buffer)
    buffs->buff_wallets_servers->ptrs = (struct pointers*) create_shared_memory(ID_SHM_WALLETS_SERVERS_PTR, sizeof(struct pointers));
    buffs->buff_wallets_servers->buffer = (struct transaction*) create_shared_memory(ID_SHM_WALLETS_SERVERS_BUFFER, sizeof(struct transaction) * info->buffers_size);

    // Buffer Servers → Main (ra_buffer)
    buffs->buff_servers_main->ptrs = (int*) create_shared_memory(ID_SHM_SERVERS_MAIN_PTR, sizeof(int) * info->buffers_size);
    buffs->buff_servers_main->buffer = (struct transaction*) create_shared_memory(ID_SHM_SERVERS_MAIN_BUFFER, sizeof(struct transaction) * info->buffers_size);

    // Arrays partilhados de info_container
    info->wallets_stats = (int*) create_shared_memory(ID_SHM_WALLETS_STATS, sizeof(int) * info->n_wallets);
    info->servers_stats = (int*) create_shared_memory(ID_SHM_SERVERS_STATS, sizeof(int) * info->n_servers);
    info->balances = (float*) create_shared_memory(ID_SHM_BALANCES, sizeof(float) * info->n_wallets);
    info->terminate = (int*) create_shared_memory(ID_SHM_TERMINATE, sizeof(int));
    // Inicializar balances
    for (int i = 0; i < info->n_wallets; i++) {
        info->balances[i] = info->init_balance;
    }

}

void destroy_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    deallocate_dynamic_memory(buffs->buff_main_wallets);
    deallocate_dynamic_memory(buffs->buff_wallets_servers);
    deallocate_dynamic_memory(buffs->buff_servers_main);
}

void destroy_shared_memory_structs(struct info_container* info, struct buffers* buffs) {
    // Buffers Main → Wallets
    destroy_shared_memory(ID_SHM_MAIN_WALLETS_PTR, buffs->buff_main_wallets->ptrs, sizeof(int) * info->buffers_size);
    destroy_shared_memory(ID_SHM_MAIN_WALLETS_BUFFER, buffs->buff_main_wallets->buffer, sizeof(struct transaction) * info->buffers_size);

    // Buffers Wallets → Servers
    destroy_shared_memory(ID_SHM_WALLETS_SERVERS_PTR, buffs->buff_wallets_servers->ptrs, sizeof(struct pointers));
    destroy_shared_memory(ID_SHM_WALLETS_SERVERS_BUFFER, buffs->buff_wallets_servers->buffer, sizeof(struct transaction) * info->buffers_size);

    // Buffers Servers → Main
    destroy_shared_memory(ID_SHM_SERVERS_MAIN_PTR, buffs->buff_servers_main->ptrs, sizeof(int) * info->buffers_size);
    destroy_shared_memory(ID_SHM_SERVERS_MAIN_BUFFER, buffs->buff_servers_main->buffer, sizeof(struct transaction) * info->buffers_size);

    // Arrays compartilhados de info_container
    destroy_shared_memory(ID_SHM_WALLETS_STATS, info->wallets_stats, sizeof(int) * info->n_wallets);
    destroy_shared_memory(ID_SHM_SERVERS_STATS, info->servers_stats, sizeof(int) * info->n_servers);
    destroy_shared_memory(ID_SHM_BALANCES, info->balances, sizeof(float) * info->n_wallets);
    destroy_shared_memory(ID_SHM_TERMINATE, info->terminate, sizeof(int));
}


void create_processes(struct info_container* info, struct buffers* buffs) {
    int i;
    for (i = 0; i < info->n_wallets; i++) {
        int pid = launch_wallet(i, info, buffs);
        info->wallets_pids[i] = pid;
    }
    for (i = 0; i < info->n_servers; i++) {
        int pid = launch_server(i, info, buffs);
        info->servers_pids[i] = pid;
    }
}


void print_balance(struct info_container* info) {
    int id;
    printf("Ingrese el id de la cartera: ");
    scanf("%d", &id);
    if (id < 0 || id >= info->n_wallets) {
        printf("Id de cartera inválido.\n");
        return;
    }
    printf("Saldo de la cartera %d: %.2f SOT\n", id, info->balances[id]);
}


void create_transaction(int* tx_counter, struct info_container* info, struct buffers* buffs) {
    if (*tx_counter >= info->max_txs) {
        printf("Número máximo de transacciones alcanzado.\n");
        return;
    }
    
    int src_id, dest_id;
    float amount;
    printf("Ingrese src_id, dest_id y amount: ");
    scanf("%d %d %f", &src_id, &dest_id, &amount);

    if (src_id < 0 || src_id >= info->n_wallets ||
        dest_id < 0 || dest_id >= info->n_wallets || amount <= 0 
        || amount > info->balances[src_id] || src_id == dest_id) {
        printf("Datos de transacción inválidos.\n");
        return;
    }
    
    struct transaction tx;
    tx.id = *tx_counter;
    tx.src_id = src_id;
    tx.dest_id = dest_id;
    tx.amount = amount;
    tx.wallet_signature = 0;
    tx.server_signature = 0;
    
    write_main_wallets_buffer(buffs->buff_main_wallets, info->buffers_size, &tx);
    printf("Transacción creada (id %d): %d -> %d, amount = %.2f\n", *tx_counter, src_id, dest_id, amount);
    (*tx_counter)++;
}

void receive_receipt(struct info_container* info, struct buffers* buffs) {
    int tx_id;
    printf("Ingrese el id de la transacción para obtener el recibo: ");
    scanf("%d", &tx_id);

    struct transaction tx;
    read_servers_main_buffer(buffs->buff_servers_main, tx_id, info->buffers_size, &tx);
    
    if (tx.id == -1) {
        printf("No se encontró recibo para la transacción %d.\n", tx_id);
    } else {
        printf("Recibo de la transacción %d:\n", tx_id);
        printf("  src_id: %d, dest_id: %d, amount: %.2f\n", tx.src_id, tx.dest_id, tx.amount);
        printf("  wallet_signature: %d, server_signature: %d\n", tx.wallet_signature, tx.server_signature);
    }
}


void print_stat(int tx_counter, struct info_container* info) {
    printf("\n--- Estadísticas Actuales ---\n");
    printf("Saldo inicial: %.2f\n", info->init_balance);
    printf("Número de carteras: %d\n", info->n_wallets);
    printf("Número de servidores: %d\n", info->n_servers);
    printf("Tamaño del buffer: %d\n", info->buffers_size);
    printf("Máximo de transacciones: %d\n", info->max_txs);
    printf("Transacciones creadas: %d\n", tx_counter);
    printf("Flag terminate: %d\n", *(info->terminate));
    
    printf("PIDs de las carteras:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Cartera %d: %d\n", i, info->wallets_pids[i]);
    }
    printf("PIDs de los servidores:\n");
    for (int i = 0; i < info->n_servers; i++) {
        printf("  Servidor %d: %d\n", i+1, info->servers_pids[i]);
    }

    printf("Saldos actuales de las carteras:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Cartera %d: %.2f SOT\n", i, info->balances[i]);
    }

    printf("Transacciones de cada cartera:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Cartera %d: %d\n", i, info->wallets_stats[i]);
    }

    printf("Transacciones de cada servidor:\n");
    for (int i = 0; i < info->n_servers; i++) {
        printf("  Servidor %d: %d\n", i+1, info->servers_stats[i]);
    }

}


void help() {
    printf("\nComandos disponibles:\n");
    printf("  bal   - Mostrar el saldo de una cartera\n");
    printf("  trx   - Crear una nueva transacción\n");
    printf("  rcp   - Obtener el recibo de una transacción\n");
    printf("  stat  - Imprimir estadísticas actuales del sistema\n");
    printf("  help  - Mostrar este mensaje de ayuda\n");
    printf("  end   - Terminar la ejecución del SOchain\n");
}

void write_final_statistics(struct info_container* info) {
    int i;
    printf("\n--- Estadísticas Finales ---\n");
    printf("Saldos finales de las carteras:\n");
    for (i = 0; i < info->n_wallets; i++) {
        printf("  Cartera %d: %.2f\n", i, info->balances[i]);
    }
    printf("Transacciones firmadas por cada cartera:\n");
    for (i = 0; i < info->n_wallets; i++) {
        printf("  Cartera %d: %d\n", i, info->wallets_stats[i]);
    }
    printf("Transacciones procesadas por cada servidor:\n");
    for (i = 0; i < info->n_servers; i++) {
        printf("  Servidor %d: %d\n", i+1, info->servers_stats[i]);
    }
}


void wait_processes(struct info_container* info) {
    int i;
    for (i = 0; i < info->n_wallets; i++) {
        wait_process(info->wallets_pids[i]);
    }
    for (i = 0; i < info->n_servers; i++) {
        wait_process(info->servers_pids[i]);
    }
}


void end_execution(struct info_container* info, struct buffers* buffs) {
    *(info->terminate) = 1;   // Indica que el sistema debe terminar
    wait_processes(info);
    write_final_statistics(info);
    printf("Cerrando SOchain...\n");
}

void user_interaction(struct info_container* info, struct buffers* buffs) {
    char comando[20];
    int tx_counter = 0;  // Contador de transacciones creadas
    
    help(); // Muestra los comandos disponibles
    
    while (1) {
        printf("\nSOchain> ");
        scanf("%s", comando);
        
        if (strcmp(comando, "bal") == 0) {
            print_balance(info);
        }
        else if (strcmp(comando, "trx") == 0) {
            create_transaction(&tx_counter, info, buffs);
        }
        else if (strcmp(comando, "rcp") == 0) {
            receive_receipt(info, buffs);
        }
        else if (strcmp(comando, "stat") == 0) {
            print_stat(tx_counter, info);
        }
        else if (strcmp(comando, "help") == 0) {
            help();
        }
        else if (strcmp(comando, "end") == 0) {
            end_execution(info, buffs);
            break;
        }
        else {
            printf("Comando desconocido. Escriba 'help' para ver los comandos disponibles.\n");
        }
    }
}


int main(int argc, char *argv[]) {
    //init data structures
    struct info_container* info = allocate_dynamic_memory(sizeof(struct info_container));
    struct buffers* buffs = allocate_dynamic_memory(sizeof(struct buffers));
    //execute main code
    
    main_args(argc, argv, info);
    create_dynamic_memory_structs(info, buffs);
    create_shared_memory_structs(info, buffs);
    create_processes(info, buffs);
    user_interaction(info, buffs);
    //release memory before terminating
    destroy_shared_memory_structs(info, buffs);
    destroy_dynamic_memory_structs(info, buffs);
    return 0;
    }
    