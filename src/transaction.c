#include <stdio.h>
#include "../inc/transaction.h"

// Função para inicializar uma transação
void init_transaction(struct transaction* trx, int id, int src_id, int dest_id, int amount) {
    trx->id = id;
    trx->src_id = src_id;
    trx->dest_id = dest_id;
    trx->amount = amount;
    trx->wallet_id = -1;  // Inicializa como não assinado
    trx->server_id = -1;  // Inicializa como não processado
    trx->valid = 0;       // Inicializa como inválido
}

// Função para imprimir as informações de uma transação
void print_transaction(struct transaction* trx) {
    printf("Transação %d:\n", trx->id);
    printf("  Origem: %d -> Destino: %d\n", trx->src_id, trx->dest_id);
    printf("  Quantidade: %d SOT\n", trx->amount);
    if (trx->wallet_id != -1) {
        printf("  Assinada pela Wallet %d\n", trx->wallet_id);
    }
    if (trx->server_id != -1) {
        printf("  Processada pelo Server %d\n", trx->server_id);
    }
    if (trx->valid) {
        printf("  Transação válida\n");
    } else {
        printf("  Transação inválida\n");
    }
}

// Função para verificar se a transação é válida
int is_transaction_valid(struct transaction* trx) {
    return trx->valid;
}
