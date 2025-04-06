// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/wallet.h"

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
    
    // inicializar a transação como "vazia"
    tx.id = -1;
    while (1) {
        // verifica se o "terminate" foi pedido
        if (*(info->terminate) == 1) {
            break;
        }
        
        //é feita uma tentativa de ler uma transação a partir do buffer Main.Wallets
        wallet_receive_transaction(&tx, wallet_id, info, buffs);
        
        // se não foi obtida uma transação válida, aguardar alguns milissegundos e depois continua
        if (tx.id == -1) {
            usleep(100000); 
            continue;
        }
        
        // processar (assinar) a transação apenas se o src_id corresponder ao da carteira
        if (tx.src_id == wallet_id) {
            wallet_process_transaction(&tx, wallet_id, info);
            transacciones_firmadas++;
            // enviar a transação assinada para o buffer Wallets->Servers
            wallet_send_transaction(&tx, info, buffs);
        }
        
        // inicia a variável de transação para a próxima iteração
        tx.id = -1;
    }
    // após a conclusão, a carteira devolve o número total de transações assinadas
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
        // aumenta o contador de transações assinadas por esta carteira
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
