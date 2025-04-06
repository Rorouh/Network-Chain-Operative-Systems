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
    
    //se iniicializa la transacción con id -1 para indicar que no hay datos válidos.
    tx.id = -1;
    
    while (1) {
        if (*(info->terminate) == 1) {
            break;
        }
        
        //intenta leer una transacción del buffer de Wallets->Servers
        server_receive_transaction(&tx, info, buffs);
        
        //si no se leyó ninguna transacción válida, esperar 100 ms y continuar
        if (tx.id == -1) {
            usleep(100000);  // Espera 100 ms
            continue;
        }
        
        //procesa la transacción: validar y actualizar saldos, firmar con el id del servidor
        server_process_transaction(&tx, server_id, info);
        
        //solo envía la transacción si fue firmada por el servidor (server_signature != 0)
        if (tx.server_signature != 0) {
            server_send_transaction(&tx, info, buffs);
            transacciones_procesadas++;
        }
        
        //reinicia tx.id a -1 para la siguiente iteración
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
    //lee del buffer circular entre Wallets y Servers.
    read_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}

/* Função que processa uma transação tx, verificando a validade dos identificadores das carteiras de origem e destino,
 * dos fundos da carteira de origem e a assinatura da carteira de origem. Atualiza os saldos das carteiras envolvidas, 
 * adiciona a assinatura do servidor e incrementa o contador de transações processadas pelo servidor.
 */
void server_process_transaction(struct transaction* tx, int server_id, struct info_container* info) {
    //Verifica que los identificadores sean válidos
    if (tx->src_id < 0 || tx->src_id >= info->n_wallets ||
        tx->dest_id < 0 || tx->dest_id >= info->n_wallets) {
        return;  //transacción inválida, las identificadores fuera de rango.
    }
    
    //Verifica que la transacción ya fue firmada por la wallet (wallet_signature debe coincidir con src_id)
    if (tx->wallet_signature != tx->src_id) {
        return;  //transacción inválida, la firma de wallet incorrecta.
    }
    
    //Verificaque la cartera de origen tenga fondos suficientes
    if (info->balances[tx->src_id] < tx->amount) {
        return;  //transacción inválida, los fondos insuficientes.
    }
    
    //procesa la transacción: actualizar saldos
    info->balances[tx->src_id] -= tx->amount;
    info->balances[tx->dest_id] += tx->amount;
    
    //firma la transacción con el id del servidor
    tx->server_signature = server_id+1;
    
    //incrementa el contador de transacciones procesadas por este servidor
    info->servers_stats[server_id]++;
}

/* Função que escreve uma transação correta processada no buffer de memória partilhada entre os servidores e a main.
 * Caso o servidor não tenha assinado a transação, essa não será escrita pois significa que a transação era inválida.
 * Se não houver espaço no buffer, a transação não é enviada e o recibo perde-se.
 */
void server_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    //verifica que la transacción tenga la firma del servidor (si no, no es válida)
    if (tx->server_signature == 0) {
        return; //La transacción no fue procesada correctamente.
    }
    //escribe la transacción en el buffer de servidores a main (ra_buffer)
    write_servers_main_buffer(buffs->buff_servers_main, info->buffers_size, tx);
}
