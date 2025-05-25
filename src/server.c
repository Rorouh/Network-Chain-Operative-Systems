// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../inc/server.h"


/* Função principal de um servidor. Deve executar um ciclo infinito onde, em 
 * cada iteração, lê uma transação do buffer de memória partilhada entre as carteiras e os servidores.
 * Se a transação tiver um id válido e info->terminate ainda for 0, o servidor valida, processa e assina 
 * a transação e a encaminha para o buffer partilhado com a main. Transações com id igual a -1 são ignoradas,
 * e aguarda-se alguns milisegundos antes de tentar ler novamente do buffer. Se info->terminate for 1, 
 * significa que o sistema deve encerrar, retornando o número de transações processadas. 
 * Pode usar os outros métodos auxiliares definidos.
 */
int execute_server(int server_id, struct info_container* info, struct buffers* buffs) {
    int transacciones_procesadas = 0;
    struct transaction tx;
    
    //     // the transaction is initialized with ID -1 to indicate there is no valid data
    tx.id = -1;
    
    while (1) {
        if (*(info->terminate) == 1) {
            break;
        }
        
        // tries to read a transaction from the Wallets->Servers buffer
        sem_wait(info->sems->wallet_server->unread);
        sem_wait(info->sems->wallet_server->mutex);
        server_receive_transaction(&tx, info, buffs);
        
        // if no valid transaction was read, wait 100 ms and continue
        if (tx.id == -1) { 
            sem_post(info->sems->wallet_server->mutex); 
            sem_post(info->sems->wallet_server->unread);           
            usleep(100000);  // Waits 100 ms
            continue;
        }
        sem_post(info->sems->wallet_server->mutex);
        sem_post(info->sems->wallet_server->free_space);
        // process the transaction: validate, update balances, and sign with the server ID
        server_process_transaction(&tx, server_id, info);
        
        // only send the transaction if it was signed by the server (server_signature != 0)
        if (tx.server_signature != 0) {
            sem_wait(info->sems->server_main->free_space);
            sem_wait(info->sems->server_main->mutex);
            server_send_transaction(&tx, info, buffs);
            sem_post(info->sems->server_main->mutex);
            sem_post(info->sems->server_main->unread);
            transacciones_procesadas++;
        }
        
        // reset tx.id to -1 for the next iteration
        tx.id = -1;
    }
    
    return transacciones_procesadas;
}

/* Função que lê uma transação do buffer de memória partilhada entre as carteiras e os servidores. Caso não
 * exista transações a serem lidas do buffer, retorna uma transação com o tx.id -1. Antes de tentar ler a 
 * transação do buffer, deve verificar se info->terminate tem valor 1. Em caso afirmativo, retorna imediatamente.
 */
void server_receive_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {

    if (*(info->terminate) == 1) {
        tx->id = -1;
        return;
    }
    // read from the circular buffer between Wallets and Servers
    read_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}

/* Função que processa uma transação tx, verificando a validade dos identificadores das carteiras de origem e destino,
 * dos fundos da carteira de origem e a assinatura da carteira de origem. Atualiza os saldos das carteiras envolvidas, 
 * adiciona a assinatura do servidor e incrementa o contador de transações processadas pelo servidor.
 */
void server_process_transaction(struct transaction* tx, int server_id, struct info_container* info) {
    // check if the IDs are valid
    if (tx->src_id < 0 || tx->src_id >= info->n_wallets ||
        tx->dest_id < 0 || tx->dest_id >= info->n_wallets) {
        return;  // invalid transaction: IDs are out of range
    }
    
    // check if the transaction has already been signed by the wallet (wallet_signature should match src_id)
    if (tx->wallet_signature != tx->src_id) {
        return;  // Invalid transaction: incorrect wallet signature
    }
    
    // check if the source wallet has sufficient funds
    if (info->balances[tx->src_id] < tx->amount) {
        return;  // invalid transaction: insufficient funds
    }
    
    // process the transaction: update balances
    info->balances[tx->src_id] -= tx->amount;
    info->balances[tx->dest_id] += tx->amount;
    
    // sign the transaction with the server ID
    tx->server_signature = server_id+1;
    
    // increment the counter of transactions processed by this server
    info->servers_stats[server_id]++;
}

/* Função que escreve uma transação correta processada no buffer de memória partilhada entre os servidores e a main.
 * Caso o servidor não tenha assinado a transação, essa não será escrita pois significa que a transação era inválida.
 * Se não houver espaço no buffer, a transação não é enviada e o recibo perde-se.
 */
void server_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    // check if the transaction has the server's signature (if not, it is invalid)

    if (tx->server_signature == 0) {
        return; // the transaction was not successfully processed
    }
    // write the transaction to the servers-main buffer (ra_buffer)
    save_time(&tx->change_time.servers);
    write_servers_main_buffer(buffs->buff_servers_main, info->buffers_size, tx);
}
