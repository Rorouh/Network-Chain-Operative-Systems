// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/wallet.h"

#include "../inc/ctime.h"
#include"../inc/main.h"
/* Função principal de uma carteira. Deve executar um ciclo infinito onde,
 * em cada iteração, lê uma transação da main apenas caso o src_id da transação seja
 * o seu próprio id. Se info->terminate ainda for 0, a carteira assina autorizando a transação, 
 * encaminhando-a para os servidores. Se a transação tiver um id igual a -1, é ignorada e espera-se 
 * alguns milisegundos antes de tentar ler uma nova transação do buffer. Se info->terminate for 1,
 * a execução termina e retorna o número de transações assinadas.
 */
int execute_wallet(int wallet_id, struct info_container* info, struct buffers* buffs) {
    int transacciones_firmadas = 0;
    struct transaction tx;
    
    // initialize the transaction as "empty"
    tx.id = -1;
    while (1) {
        // checks if "terminate" was requested
        if (*(info->terminate) == 1) {
            break;
        }
        
        // an attempt is made to read a transaction from the Main->Wallets buffer
        
        sem_wait(info->sems->main_wallet->unread);
        sem_wait(info->sems->main_wallet->mutex);
        wallet_receive_transaction(&tx, wallet_id, info, buffs);
        
        // if a valid transaction was not obtained, wait a few milliseconds and then continue
        if (tx.id == -1) {
            sem_post(info->sems->main_wallet->unread);
            sem_post(info->sems->main_wallet->mutex);
            usleep(100000); 
            continue; 
        }
        sem_post(info->sems->main_wallet->mutex);        
        sem_post(info->sems->main_wallet->free_space);
        // process (sign) the transaction only if the src_id matches the wallet's id
        if (tx.src_id == wallet_id) {
            
            // send the signed transaction to the Wallets->Servers buffer
            save_time(&tx.change_time.wallets);
            sem_wait(info->sems->wallet_server->free_space);
            sem_wait(info->sems->wallet_server->mutex);
            wallet_process_transaction(&tx, wallet_id, info);
            transacciones_firmadas++;  
            wallet_send_transaction(&tx, info, buffs);
            sem_post(info->sems->wallet_server->mutex);        
            sem_post(info->sems->wallet_server->unread);
        }
        
        // initialize the transaction variable for the next iteration
        tx.id = -1;
    }
    // after completion, the wallet returns the total number of signed transactions
    return transacciones_firmadas;
}

/* Função que lê uma transação do buffer de memória partilhada entre a main e as carteiras apenas 
 * caso o src_id da transação seja o seu próprio id. Antes de tentar ler a transação, deve verificar 
 * se info->terminate tem valor 1. Em caso afirmativo, retorna imediatamente da função.
 */
void wallet_receive_transaction(struct transaction* tx, int wallet_id, struct info_container* info, struct buffers* buffs) {
    if (*(info->terminate) == 1) {
        return;
    }

    read_main_wallets_buffer(buffs->buff_main_wallets, wallet_id, info->buffers_size, tx);
}

/* Função que assina uma transação comprovando que a carteira de origem src_id da transação corresponde
 * ao wallet_id. Atualiza o campo wallet_signature da transação e incrementa o contador de transações 
 * assinadas pela carteira.
 */
void wallet_process_transaction(struct transaction* tx, int wallet_id, struct info_container* info) {
    if (tx->src_id == wallet_id) {
        tx->wallet_signature = wallet_id;
        // increases the counter of transactions signed by this wallet
        info->wallets_stats[wallet_id]++;
    }
    
}

/* Função que escreve uma transação assinada no buffer de memória partilhada entre
 * as carteiras e os servidores. Se não houver espaço disponível no buffer, a transação
 * perde-se.
 */
void wallet_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    write_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}
