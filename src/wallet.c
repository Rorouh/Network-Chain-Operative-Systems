// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/wallet.h"


void wallet_receive_transaction(struct transaction* tx, int wallet_id, struct info_container* info, struct buffers* buffs) {
    //comprobar si se ha solicitado la terminación
    if (*(info->terminate) == 1) {
        return;
    }
    //leee una transacción del buffer entre Main y Wallets
    read_main_wallets_buffer(buffs->buff_main_wallets, wallet_id, info->buffers_size, tx);
}


void wallet_process_transaction(struct transaction* tx, int wallet_id, struct info_container* info) {
    if (tx->src_id == wallet_id) {
        tx->wallet_signature = wallet_id;
        //incrementa el contador de transacciones firmadas por esta cartera
        info->wallets_stats[wallet_id]++;
    }
}

void wallet_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    write_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}

int execute_wallet(int wallet_id, struct info_container* info, struct buffers* buffs) {
    int transacciones_firmadas = 0;
    struct transaction tx;
    
    //inicializar la transacción como "vacía"
    tx.id = -1;
    while (1) {
        //se verifica si se ha solicitado el "terminate"
        if (*(info->terminate) == 1) {
            break;
        }
        
        //se intenta leer una transacción desde el buffer de Main.Wallets
        wallet_receive_transaction(&tx, wallet_id, info, buffs);
        
        //si no se obtuvo una transacción válida, esperar unos milisegundos y continuar
        if (tx.id == -1) {
            usleep(100000); 
            continue;
        }
        
        //procesar(firmar) la transacción solo si el src_id coincide
        if (tx.src_id == wallet_id) {
            wallet_process_transaction(&tx, wallet_id, info);
            transacciones_firmadas++;
            // Enviar la transacción firmada al buffer de Wallets->Servers
            wallet_send_transaction(&tx, info, buffs);
        }
        
        //einiciar la variable de transacción para la siguiente iteración
        tx.id = -1;
    }
    //al finalizar, la wallet devuelve el número de transacciones firmadas totales
    return transacciones_firmadas;
}
