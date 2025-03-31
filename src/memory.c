// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include "../inc/memory.h"

void* allocate_dynamic_memory(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        perror("Falha ao alocar memória dinâmica");
        exit(1);
    }
    return ptr;
}

void create_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    // Aloca a memória dinâmica para as estruturas
    info->wallets = (struct wallet*)malloc(sizeof(struct wallet) * info->n_wallets);
    buffs->buffer_wallets = (struct transaction*)malloc(sizeof(struct transaction) * info->buff_size);
}

void destroy_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    // Libera a memória dinâmica
    free(info->wallets);
    free(buffs->buffer_wallets);
}
