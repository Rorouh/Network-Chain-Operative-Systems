// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/memory.h"
#include "../inc/process.h"
#include "../inc/wallet.h"
#include "../inc/server.h"

int main(int argc, char *argv[]) {
    // Inicialização das estruturas de dados
    struct info_container* info = allocate_dynamic_memory(sizeof(struct info_container));
    struct buffers* buffs = allocate_dynamic_memory(sizeof(struct buffers));

    // Inicialização das variáveis passadas pela linha de comandos
    main_args(argc, argv, info);

    // Criar estruturas de memória dinâmica e partilhada
    create_dynamic_memory_structs(info, buffs);
    create_shared_memory_structs(info, buffs);

    // Criar os processos para Wallets e Servers
    create_processes(info, buffs);

    // Interação com o usuário
    user_interaction(info, buffs);

    // Liberação da memória antes de terminar
    destroy_shared_memory_structs(info, buffs);
    destroy_dynamic_memory_structs(info, buffs);

    return 0;
}
