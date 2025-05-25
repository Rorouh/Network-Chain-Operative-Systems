// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>
#include "../inc/synchronization.h"

/* Function that creates *one* semaphore, initialized to <v> */
sem_t* create_semaphore(char *name, unsigned v){
    sem_unlink(name); // should this be needed? ensures it doesent exist previously
    sem_t *s;
    s = sem_open(name, O_CREAT, 0644, v);

    if(s == SEM_FAILED){
        perror(name);
        exit(6);
    }

    return s;
}

/* Function to close/destroy a semaphore, given its name and pointer */
void destroy_semaphore(char *name, sem_t *sem) {
    sem_close(sem);
    sem_unlink(name);
}

/* Generic function that creates 3 semaphores used in the Producer-Consumer logic
1st argument: v - initial value for the free_space semaphore
Remaining arguments: the 3 names to give to the semaphores->
Returns: a pointer to the structure that contains 3 semaphores-> */
struct triplet_sems* create_triplet_sems(unsigned v, char *freespace_name1, char *unread_name, char *mutex_name) {
    
    struct triplet_sems *t = malloc(sizeof(struct triplet_sems));

    t->free_space = create_semaphore(freespace_name1, v);
    t->unread = create_semaphore(unread_name, 0);
    t->mutex = create_semaphore(mutex_name, 1);

    return t;
}

/* Function that creates the 3 semaphores needed to access the Main-Wallet buffer */
struct triplet_sems* create_main_wallet_sems(unsigned v) {
    return create_triplet_sems(v, STR_SEM_MAIN_WALLET_FREESPACE, STR_SEM_MAIN_WALLET_UNREAD, STR_SEM_MAIN_WALLET_MUTEX);
}

/* Function that creates the 3 semaphores needed to access the Wallet-Server buffer */
struct triplet_sems* create_wallet_server_sems(unsigned v) {
    return create_triplet_sems(v, STR_SEM_WALLET_SERVER_FREESPACE, STR_SEM_WALLET_SERVER_UNREAD, STR_SEM_WALLET_SERVER_MUTEX);
}

/* Function that creates the 3 semaphores needed to access the Server-Main buffer */
struct triplet_sems* create_server_main_sems(unsigned v) {
    return create_triplet_sems(v,
        STR_SEM_SERVER_MAIN_FREESPACE, STR_SEM_SERVER_MAIN_UNREAD, STR_SEM_SERVER_MAIN_MUTEX);
}

/* Function that creates *all* program semaphores, initializing the free_space semaphores to <v> */
struct semaphores* create_all_semaphores(unsigned v) {
    struct semaphores *s = malloc(sizeof(struct semaphores));

    s->main_wallet = create_main_wallet_sems(v);
    s->wallet_server = create_wallet_server_sems(v);
    s->server_main = create_server_main_sems(v);
    s->terminate_mutex = create_semaphore(STR_SEM_TERMINATE_MUTEX, 1);

    return s;
}

/* Function that destroys *all* semaphores in the <sems> structure */
void destroy_all_semaphores(struct semaphores* s) {
    // Main-> Wallets
    destroy_semaphore(STR_SEM_MAIN_WALLET_FREESPACE, s->main_wallet->free_space);
    destroy_semaphore(STR_SEM_MAIN_WALLET_UNREAD,   s->main_wallet->unread);
    destroy_semaphore(STR_SEM_MAIN_WALLET_MUTEX,    s->main_wallet->mutex);
    free(s->main_wallet);

    // Wallets->Servers
    destroy_semaphore(STR_SEM_WALLET_SERVER_FREESPACE, s->wallet_server->free_space);
    destroy_semaphore(STR_SEM_WALLET_SERVER_UNREAD,    s->wallet_server->unread);
    destroy_semaphore(STR_SEM_WALLET_SERVER_MUTEX,     s->wallet_server->mutex);
    free(s->wallet_server);

    // Servers -> Main
    destroy_semaphore(STR_SEM_SERVER_MAIN_FREESPACE, s->server_main->free_space);
    destroy_semaphore(STR_SEM_SERVER_MAIN_UNREAD,    s->server_main->unread);
    destroy_semaphore(STR_SEM_SERVER_MAIN_MUTEX,     s->server_main->mutex);
    free(s->server_main);

    // Terminate mutex
    destroy_semaphore(STR_SEM_TERMINATE_MUTEX, s->terminate_mutex);

  
    free(s);
}


/* Print the value of *all* semaphores in <sems> */
void print_all_semaphores(struct semaphores* s) {
    int v;

	// main -> wallets: wallets read transactions that main writes
    sem_getvalue(s->main_wallet->free_space, &v);
    printf("Main Wallet freespace: %d\n", v);
    sem_getvalue(s->main_wallet->unread, &v);
    printf("Main Wallet unread: %d\n", v);
    sem_getvalue(s->main_wallet->mutex, &v);
    printf("Main Wallet mutex: %d\n", v);

	// wallets -> servers: transactions signed by wallets
    sem_getvalue(s->wallet_server->free_space, &v);
    printf("Wallet Server freespace: %d\n", v);
    sem_getvalue(s->wallet_server->unread, &v);
    printf("Wallet Server unread: %d\n", v);
    sem_getvalue(s->wallet_server->mutex, &v);
    printf("Wallet Server mutex: %d\n", v);

	// servers -> main: receipts of transactions returned by servers
    sem_getvalue(s->server_main->free_space, &v);
    printf("Server Main freespace: %d\n", v);
    sem_getvalue(s->server_main->unread, &v);
    printf("Server Main unread: %d\n", v);
    sem_getvalue(s->server_main->mutex, &v);
    printf("Server Main mutex: %d\n", v);

    sem_getvalue(s->terminate_mutex, &v);
    printf("Terminate mutex: %d\n", v);
}
