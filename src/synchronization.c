#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>
#include "../inc/synchronization.h"

/* Função que cria *um* semaforo , inicializado a <vue> */
sem_t* create_semaphore(char *name, unsigned v){
    // sem_unlink(name); // should this be needed? ensures it doesent exist previously
    sem_t *s;
    s = sem_open(name, O_CREAT, 0644, v);

    if(s == SEM_FAILED){
        perror(name);
        exit(6);
    }

    return s;
}

/* Função para desligar/destruir um semaforo , em funcão do seu nome e pointer */
void destroy_semaphore(char *name, sem_t *sem) {
    sem_close(sem);
    sem_unlink(name);
}

/* função genérica que cria 3 semaforos usados na lógica Produtor-Consumidor 
1º argumento: v - vor inicial do semaforo free_space
Restantes argumentos: os 3 nomes a dar aos semáforos->
Retorna: um pointer para a estrutura que contem 3 semaforos-> */
struct triplet_sems* create_triplet_sems(unsigned v, char *freespace_name1, char *unread_name, char *mutex_name) {
    
    struct triplet_sems *t = malloc(sizeof(struct triplet_sems));

    t->free_space = create_semaphore(freespace_name1, v);
    t->unread = create_semaphore(unread_name, 0);
    t->mutex = create_semaphore(mutex_name, 1);

    return t;
}

/* funcao que cria os 3 semaforos necessários para aceder ao buffer Main-Wallet */
struct triplet_sems* create_main_wallet_sems(unsigned v) {
    return create_triplet_sems(v, STR_SEM_MAIN_WALLET_FREESPACE, STR_SEM_MAIN_WALLET_UNREAD, STR_SEM_MAIN_WALLET_MUTEX);
}

/* funcao que cria os 3 semaforos necessários para aceder ao buffer Wallet-Server */
struct triplet_sems* create_wallet_server_sems(unsigned v) {
    return create_triplet_sems(v, STR_SEM_WALLET_SERVER_FREESPACE, STR_SEM_WALLET_SERVER_UNREAD, STR_SEM_WALLET_SERVER_MUTEX);
}

/* funcao que cria os 3 semaforos necessários para aceder ao buffer Server-Main */
struct triplet_sems* create_server_main_sems(unsigned v) {
    return create_triplet_sems(v,
        STR_SEM_SERVER_MAIN_FREESPACE, STR_SEM_SERVER_MAIN_UNREAD, STR_SEM_SERVER_MAIN_MUTEX);
}

/* Função que cria *todos* os semaforos do programa, inicializando a <v> os semaforos free_space */
struct semaphores* create_all_semaphores(unsigned v) {
    struct semaphores *s = malloc(sizeof(struct semaphores));

    s->main_wallet = create_main_wallet_sems(v);
    s->wallet_server = create_wallet_server_sems(v);
    s->server_main = create_server_main_sems(v);
    s->terminate_mutex = create_semaphore(STR_SEM_TERMINATE_MUTEX, 1);

    return s;
}

/* Função que destroi *todos* os semaforos na estrutura <sems> */
void destroy_all_semaphores(struct semaphores* s) {
    destroy_semaphore(STR_SEM_MAIN_WALLET_FREESPACE, s->main_wallet->free_space);
    destroy_semaphore(STR_SEM_MAIN_WALLET_UNREAD, s->main_wallet->unread);
    destroy_semaphore(STR_SEM_MAIN_WALLET_MUTEX, s->main_wallet->mutex);
    
    destroy_semaphore(STR_SEM_WALLET_SERVER_FREESPACE, s->wallet_server->free_space);
    destroy_semaphore(STR_SEM_WALLET_SERVER_UNREAD, s->wallet_server->unread);
    destroy_semaphore(STR_SEM_WALLET_SERVER_MUTEX, s->wallet_server->mutex);
    
    destroy_semaphore(STR_SEM_SERVER_MAIN_FREESPACE, s->server_main->free_space);
    destroy_semaphore(STR_SEM_SERVER_MAIN_UNREAD, s->server_main->unread);
    destroy_semaphore(STR_SEM_SERVER_MAIN_MUTEX, s->server_main->mutex);

    destroy_semaphore(STR_SEM_TERMINATE_MUTEX, s->terminate_mutex);
    
    free(s->main_wallet);
    free(s->wallet_server);
    free(s->server_main);
    free(s);
}

/* Imprimir o vor de *todos* os semaforos em <sems> */
void print_all_semaphores(struct semaphores* s) {
    int v;

	// main -> wallets: wallets lê transacções que a main escreve
    sem_getvalue(s->main_wallet->free_space, &v);
    printf("Main Wallet freespace: %d\n", v);
    sem_getvalue(s->main_wallet->unread, &v);
    printf("Main Wallet unread: %d\n", v);
    sem_getvalue(s->main_wallet->mutex, &v);
    printf("Main Wallet mutex: %d\n", v);

	// wallets -> servers: transações assinadas pelas wallets
    sem_getvalue(s->wallet_server->free_space, &v);
    printf("Wallet Server freespace: %d\n", v);
    sem_getvalue(s->wallet_server->unread, &v);
    printf("Wallet Server unread: %d\n", v);
    sem_getvalue(s->wallet_server->mutex, &v);
    printf("Wallet Server mutex: %d\n", v);

	// servers -> main:	recibos de transações devolvidos pelos servers
    sem_getvalue(s->server_main->free_space, &v);
    printf("Server Main freespace: %d\n", v);
    sem_getvalue(s->server_main->unread, &v);
    printf("Server Main unread: %d\n", v);
    sem_getvalue(s->server_main->mutex, &v);
    printf("Server Main mutex: %d\n", v);

    sem_getvalue(s->terminate_mutex, &v);
    printf("Terminate mutex: %d\n", v);
}
