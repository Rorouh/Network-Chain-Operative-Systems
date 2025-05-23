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
    
    // a transação é iniciada com ID -1 para indicar que não há dados válidos
    tx.id = -1;
    
    while (1) {
        if (*(info->terminate) == 1) {
            break;
        }
        
        // tenta ler uma transação do buffer Wallets->Servers
        server_receive_transaction(&tx, info, buffs);
        
        // se nenhuma transação válida foi lida, aguarde 100 ms e continue
        if (tx.id == -1) {
            usleep(100000);  // Espera 100 ms
            continue;
        }
        
        // processar a transação: valida , atualiza saldos e assina com o ID do servidor
        server_process_transaction(&tx, server_id, info);
        
        // só envia a transação se ela foi assinada pelo servidor (server_signature != 0)
        if (tx.server_signature != 0) {
            server_send_transaction(&tx, info, buffs);
            transacciones_procesadas++;
        }
        
        // eedefina tx.id para -1 para a próxima iteração
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
    // leitura do buffer circular entre Carteiras e Servidores
    read_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}

/* Função que processa uma transação tx, verificando a validade dos identificadores das carteiras de origem e destino,
 * dos fundos da carteira de origem e a assinatura da carteira de origem. Atualiza os saldos das carteiras envolvidas, 
 * adiciona a assinatura do servidor e incrementa o contador de transações processadas pelo servidor.
 */
void server_process_transaction(struct transaction* tx, int server_id, struct info_container* info) {
    // verifica se os identificadores são válidos
    if (tx->src_id < 0 || tx->src_id >= info->n_wallets ||
        tx->dest_id < 0 || tx->dest_id >= info->n_wallets) {
        return;  // transação inválida: os identificadores estão fora do intervalo
    }
    
    // verifica se a transação já foi assinada pela carteira (wallet_signature deve corresponder src_id)
    if (tx->wallet_signature != tx->src_id) {
        return;  // Transação inválida: assinatura de carteira incorreta
    }
    
    // verifica se a carteira de origem tem fundos suficientes
    if (info->balances[tx->src_id] < tx->amount) {
        return;  // transação inválida: fundos insuficientes
    }
    
    // processar a transação: atualizar saldos
    info->balances[tx->src_id] -= tx->amount;
    info->balances[tx->dest_id] += tx->amount;
    
    // assina a transação com o ID do servidor
    tx->server_signature = server_id+1;
    
    // incrementa o contador de transações processadas por este servidor
    info->servers_stats[server_id]++;
}

/* Função que escreve uma transação correta processada no buffer de memória partilhada entre os servidores e a main.
 * Caso o servidor não tenha assinado a transação, essa não será escrita pois significa que a transação era inválida.
 * Se não houver espaço no buffer, a transação não é enviada e o recibo perde-se.
 */
void server_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    // verifica se a transação tem a assinatura do servidor (se não, é inválida)
    if (tx->server_signature == 0) {
        return; // a transação não foi processada com sucesso
    }
    // grava a transação no buffer de servidores em Main (ra_buffer)
    save_time(&tx->change_time.servers);
    write_servers_main_buffer(buffs->buff_servers_main, info->buffers_size, tx);
}
