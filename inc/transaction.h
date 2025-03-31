#ifndef TRANSACTION_H
#define TRANSACTION_H

// Definição da estrutura para uma transação
struct transaction {
    int id;              // Identificador único da transação
    int src_id;          // ID da carteira de origem
    int dest_id;         // ID da carteira de destino
    int amount;          // Quantidade de SOT transferida
    int wallet_id;       // ID da Wallet que assinou a transação
    int server_id;       // ID do servidor que processou a transação
    int valid;           // Flag para verificar se a transação é válida
};

// Funções para manipulação das transações
void init_transaction(struct transaction* trx, int id, int src_id, int dest_id, int amount);
void print_transaction(struct transaction* trx);
int is_transaction_valid(struct transaction* trx);

#endif // TRANSACTION_H
