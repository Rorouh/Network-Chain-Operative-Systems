// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include "../inc/memory.h"
#include "../inc/process.h"
#include "../inc/wallet.h"
#include "../inc/server.h"
#include "../inc/synchronization.h"
#include "../inc/csettings.h"
#include "../inc/clog.h"
#include "../inc/ctime.h"
#include "../inc/main.h"
#include "../inc/csignals.h"
#include "../inc/cstats.h"
#include <termios.h>

int main_rcp = 0;
int main_trx = 0;
int period = 0;
char log_filename [50];
char statistics_filename[50];
/* Função que lê do stdin com o scanf apropriado para cada tipo de dados
 * e valida os argumentos da aplicação, incluindo o saldo inicial, 
 * o número de carteiras, o número de servidores, o tamanho dos buffers 
 * e o número máximo de transações. Guarda essa informação na estrutura info_container.
 */

void main_args(int argc, char *argv[], struct info_container *info) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s init_balance n_wallets n_servers buffers_size max_txs\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // transformação de valores e atribuí-los para as informações
    info->init_balance = atof(argv[1]);
    info->n_wallets = atoi(argv[2]);
    info->n_servers = atoi(argv[3]);
    info->buffers_size = atoi(argv[4]);
    info->max_txs = atoi(argv[5]);

    // verificamos se os valores são pelo menos superiores a 0, caso contrário não os aceitamos
    if (info->init_balance < 0 || info->n_wallets <= 0 || info->n_servers <= 0 ||
        info->buffers_size <= 0 || info->max_txs <= 0) {
        fprintf(stderr, "Error: Argumentos inválidos.\n");
        exit(EXIT_FAILURE);
    }
}

/* Função que reserva a memória dinâmica necessária, por exemplo, 
 * para os arrays *_pids de info_container. Para tal, pode ser usada
 * a função allocate_dynamic_memory do memory.h.
 */
void create_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    info->wallets_pids = (int*) allocate_dynamic_memory(sizeof(int) * info->n_wallets);
    info->servers_pids = (int*) allocate_dynamic_memory(sizeof(int) * info->n_servers);

    buffs->buff_main_wallets = (struct ra_buffer*) allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_wallets_servers = (struct circ_buffer*) allocate_dynamic_memory(sizeof(struct circ_buffer));
    buffs->buff_servers_main = (struct ra_buffer*) allocate_dynamic_memory(sizeof(struct ra_buffer));

}

/* Função que reserva a memória partilhada necessária para a execução
 * do SOchain, incluindo buffers e arrays partilhados. É necessário
 * reservar memória partilhada para todos os buffers da estrutura
 * buffers, incluindo tanto os buffers em si como os respetivos
 * pointers, assim como para os arrays *_stats, balances e a variável
 * terminate do info_container. Pode ser usada a função
 * create_shared_memory do memory.h.
 */
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

/* Liberta a memória dinâmica previamente reservada. Pode utilizar a
 * função deallocate_dynamic_memory do memory.h
 */
void destroy_dynamic_memory_structs(struct info_container* info, struct buffers* buffs) {
    // liberar os arrays de PIDs
    deallocate_dynamic_memory(info->wallets_pids);
    deallocate_dynamic_memory(info->servers_pids);

    // liberar os buffers
    deallocate_dynamic_memory(buffs->buff_main_wallets);
    deallocate_dynamic_memory(buffs->buff_wallets_servers);
    deallocate_dynamic_memory(buffs->buff_servers_main);
}

/* Liberta a memória partilhada previamente reservada. Pode utilizar a
 * função destroy_shared_memory do memory.h
 */
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

/* Função que cria os processos das carteiras e servidores. 
 * Os PIDs resultantes são armazenados nos arrays apropriados 
 * da estrutura info_container.
 */
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

/* Função responsável pela interação com o utilizador. 
 * Permite o utilizador pedir para visualizar saldos, criar
 * transações, consultar recibos, ver estatísticas do sistema e
 * encerrar a execução.
 */
void user_interaction(struct info_container* info, struct buffers* buffs) {
    char comando[20];
    int tx_counter = 0;  // contador para transações criadas
    
    help(); // exibe os comandos disponíveis
    
    while (1) {
        printf("\nSOchain> ");
        fflush(stdout);
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
            save_operation("stat", log_filename);
        }
        else if (strcmp(comando, "help") == 0) {
            help();
            save_operation("help", log_filename);
        }
        else if (strcmp(comando, "end") == 0) {
            end_execution(info, buffs);
        }
        else if(strcmp(comando,"sem") == 0){
            printf("semaforos: \n");
            print_all_semaphores(info->sems);
        }
        else {
            printf("Comando desconhecido. Digite 'help' para ver os comandos disponíveis.\n");
        }
    }
}

/* Função que imprime as estatísticas finais do SOchain, incluindo 
 * o número de transações assinadas por cada carteira e processadas
 * por cada servidor.
 */
void write_final_statistics(struct info_container* info) {
    int i;
    printf("\n--- Estatísticas Finais ---\n");
    printf("Saldos finais das carteiras:\n");
    for (i = 0; i < info->n_wallets; i++) {
        printf("  Carteira %d: %.2f\n", i, info->balances[i]);
    }
    printf("Transações assinadas por cada carteira:\n");
    for (i = 0; i < info->n_wallets; i++) {
        printf("  Carteira %d: %d\n", i, info->wallets_stats[i]);
    }
    printf("Transações processadas por cada servidor:\n");
    for (i = 0; i < info->n_servers; i++) {
        printf("  Servidor %d: %d\n", i+1, info->servers_stats[i]);
    }
}

/* Termina a execução do programa. Deve atualizar a flag terminate e,
 * em seguida, aguardar a terminação dos processos filhos, escrever as 
 * estatísticas finais e retornar.
 */
void end_execution(struct info_container* info, struct buffers* buffs) {
    *(info->terminate) = 1;   // Indica que el sistema debe ser encerrado
    wakeup_processes(info);
    wait_processes(info);

    write_final_statistics(info);

    write_statistics(info,statistics_filename, main_trx, main_rcp);

    printf("Encerrando SOchain...\n");
    save_operation("end", log_filename);

    // destruir tudo
    destroy_shared_memory_structs(info, buffs);
    destroy_dynamic_memory_structs(info, buffs);
    destroy_all_semaphores(info->sems);

    // liberar os structs principais
    free(info);
    free(buffs);
    
    exit(1);
}

/* Aguarda a terminação dos processos filhos previamente criados. Pode usar
 * a função wait_process do process.h
 */
void wait_processes(struct info_container* info) {
    int i;
    for (i = 0; i < info->n_wallets; i++) {
        wait_process(info->wallets_pids[i]);
    }
    for (i = 0; i < info->n_servers; i++) {
        wait_process(info->servers_pids[i]);
    }
}

/* Imprime o saldo atual de uma carteira identificada pelo id que ainda está
 * no stdin a espera de ser lido com o scanf dentro da função
 */
void print_balance(struct info_container* info) {
    int id;
    printf("Introduza o ID da carteira: ");
    scanf("%d", &id);
    if (id < 0 || id >= info->n_wallets) {
        printf("ID de carteira inválido.\n");
        return;
    }
    printf("Saldo da carteira %d: %.2f SOT\n", id, info->balances[id]);
    char op[32];
    snprintf(op, sizeof(op), "bal %d", id);
    save_operation(op, log_filename);
}

/* Cria uma nova transação com os dados inseridos pelo utilizador na linha de
 * comandos (e que ainda estão no stdin a espera de serem lidos com o scanf
 * dentro da função), escreve-a no buffer de memória partilhada entre a main 
 * e as carteiras e atualiza o contador de transações criadas tx_counter. Caso
 * a aplicação já tenha atingido o número máximo de transações permitidas
 * a função retorna apenas uma mensagem de erro e não cria a nova transação.
 */
void create_transaction(int* tx_counter, struct info_container* info, struct buffers* buffs) {
    if (*tx_counter >= info->max_txs) {
        printf("Número máximo de transações alcançadas.\n");
        return;
    }
    
    int src_id, dest_id;
    float amount;
    printf("Insira src_id, dest_id e valor: ");
    scanf("%d %d %f", &src_id, &dest_id, &amount);

    if (src_id < 0 || src_id >= info->n_wallets ||
        dest_id < 0 || dest_id >= info->n_wallets || amount <= 0 
        || amount > info->balances[src_id] || src_id == dest_id) {
        printf("Dados de transação inválidos.\n");
        return;
    }

    int is = 0;
    for(int i = 0; i < info->buffers_size; i++){
        if(buffs->buff_main_wallets->ptrs[i] == 0){
            is = 1;
        }
    }
    if(is == 0){
        printf("Buffer main_wallet lleno");
        return;
    }
    struct transaction tx;
    tx.id = *tx_counter;
    tx.src_id = src_id;
    tx.dest_id = dest_id;
    tx.amount = amount;
    tx.wallet_signature = 0;
    tx.server_signature = 0;
    
    
    save_time(&tx.change_time.main);
    sem_wait(info->sems->main_wallet->free_space);
    write_main_wallets_buffer(buffs->buff_main_wallets, info->buffers_size, &tx);
    sem_post(info->sems->main_wallet->unread);
    printf("Transação criada (id %d): %d -> %d, amount = %.2f\n", *tx_counter, src_id, dest_id, amount);
    (*tx_counter)++;
    main_trx++;
    
    char op[32];
    snprintf(op, sizeof(op), "trx %d %d %.2f", src_id, dest_id, amount);
    save_operation(op, log_filename);
}

/* Tenta ler o recibo da transação (identificada por id, o qual ainda está no
 * stdin a espera de ser lido dentro da função com o scanf) do buffer de memória
 * partilhada entre os servidores e a main, comprovando a conclusão da transação.
 */


void receive_receipt(struct info_container* info, struct buffers* buffs) {
    int tx_id;
    printf("Insira o ID da transação para obter o comprovativo: ");
    scanf("%d", &tx_id);

    struct transaction tx;

    int is = 0;
    for(int i = 0; i < info->buffers_size; i++){
        if(buffs->buff_servers_main->ptrs[i] == 1){
            struct transaction t = buffs->buff_servers_main->buffer[i];
            if(t.id == tx_id){
                is = 1;
            }
        }
    }
    if (is == 0){
        printf("El recibo no ha llegado al server o no existe");
        return;
    }
    sem_wait(info->sems->server_main->unread);
    sem_wait(info->sems->server_main->mutex);
    read_servers_main_buffer(buffs->buff_servers_main, tx_id, info->buffers_size, &tx);
    
    if (tx.id == -1) {
        sem_post(info->sems->server_main->mutex);
        sem_post(info->sems->server_main->unread);
        printf("Nenhum comprovativo encontrado para a transação %d.\n", tx_id);
    } else {
        sem_post(info->sems->server_main->mutex);
        sem_post(info->sems->server_main->free_space);
        main_rcp++;
        printf("Recibo da transação %d:\n", tx_id);
        printf("  src_id: %d, dest_id: %d, amount: %.2f\n", tx.src_id, tx.dest_id, tx.amount);
        printf("  wallet_signature: %d, server_signature: %d\n", tx.wallet_signature, tx.server_signature);
    }
    
    char op[32];
    snprintf(op, sizeof(op), "rcp %d", tx_id);
    save_operation(op, log_filename);
}
/* Imprime as estatísticas atuais do sistema, incluindo as configurações iniciais
 * do sistema, o valor das variáveis terminate e contador da transações criadas,
 * os pids dos processos e as restantes informações (e.g., número de transações 
 * assinadas pela entidade e saldo atual no caso das carteiras).
 */
void print_stat(int tx_counter, struct info_container* info) {
    printf("\n--- Estatísticas Atuais ---\n");
    printf("Saldo inicial: %.2f\n", info->init_balance);
    printf("Número de carteiras: %d\n", info->n_wallets);
    printf("Número de servidores: %d\n", info->n_servers);
    printf("Buffer size: %d\n", info->buffers_size);
    printf("Máximo de transações: %d\n", info->max_txs);
    printf("Transações criadas: %d\n", tx_counter);
    printf("Flag terminate: %d\n", *(info->terminate));
    
    printf("PIDs das carteiras:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Carteira %d: %d\n", i, info->wallets_pids[i]);
    }
    printf("PIDs dos servidores:\n");
    for (int i = 0; i < info->n_servers; i++) {
        printf("  Servidor %d: %d\n", i+1, info->servers_pids[i]);
    }

    printf("Saldos actuais das carteiras:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Carteira %d: %.2f SOT\n", i, info->balances[i]);
    }

    printf("Transações de cada carteira:\n");
    for (int i = 0; i < info->n_wallets; i++) {
        printf("  Carteira %d: %d\n", i, info->wallets_stats[i]);
    }

    printf("Transações de cada servidor:\n");
    for (int i = 0; i < info->n_servers; i++) {
        printf("  Servidor %d: %d\n", i+1, info->servers_stats[i]);
    }

}

/* Exibe informações sobre os comandos disponíveis na aplicação. 
 */
void help() {
    printf("\nComandos disponíveis:\n");
    printf("  bal   - Mostrar saldo da carteira\n");
    printf("  trx   - Criar uma nova transação\n");
    printf("  rcp   - Obter um recibo de uma transação\n");
    printf("  stat  - Imprimir estatísticas atuais do sistema\n");
    printf("  help  - Mostrar esta mensagem de ajuda\n");
    printf("  end   - Terminar a execução da SOchain\n");

}

void wakeup_processes(struct info_container* info) {
    for (int i = 0; i < info->n_wallets; i++) {
        sem_post(info->sems->main_wallet->unread);
        sem_post(info->sems->main_wallet->free_space);
        sem_post(info->sems->main_wallet->mutex);
        sem_post(info->sems->wallet_server->unread);
        sem_post(info->sems->wallet_server->free_space);
        sem_post(info->sems->wallet_server->mutex);
    }
    for (int i = 0; i < info->n_servers; i++) {
        sem_post(info->sems->wallet_server->unread);
        sem_post(info->sems->wallet_server->free_space);
        sem_post(info->sems->wallet_server->mutex);
        sem_post(info->sems->server_main->unread);
        sem_post(info->sems->server_main->free_space);
        sem_post(info->sems->server_main->mutex);
    }
    for (int i = 0; i < info->n_wallets; i++) {
        sem_post(info->sems->wallet_server->unread);
        sem_post(info->sems->wallet_server->free_space);
    }
    for (int i = 0; i < info->n_servers; i++) {
        sem_post(info->sems->wallet_server->unread);
        sem_post(info->sems->wallet_server->free_space);
    }
    for (int i = 0; i < info->n_servers; i++) {
        sem_post(info->sems->server_main->free_space);
    }

}

/* Função principal do SOchain. Inicializa o sistema, chama as funções de alocação
 * de memória, a de criação de processos filhos, a de interação do utilizador 
 * e aguarda o encerramento dos processos para chamar as funções para libertar 
 * a memória alocada.
 */
int main(int argc, char *argv[]) {
    //init data structures
    struct info_container* info = allocate_dynamic_memory(sizeof(struct info_container));
    struct buffers* buffs = allocate_dynamic_memory(sizeof(struct buffers));
    //execute main code
    //main_args(argc, argv, info);
    if(argc != 3) {
        printf("Numero de argumentos inválido.\n");
        return 1;
    }

    read_args(argv[1], info);
    read_settings(argv[2], log_filename,statistics_filename, &period);
    info->sems = create_all_semaphores(info->buffers_size);

    create_dynamic_memory_structs(info, buffs);
    create_shared_memory_structs(info, buffs);
    setup_ctrlC_signal(info, buffs);
    create_processes(info, buffs);
    setup_ctrlC_signal_parent();
    setup_alarm(period);
    
    user_interaction(info, buffs);
    return 0;
}
    