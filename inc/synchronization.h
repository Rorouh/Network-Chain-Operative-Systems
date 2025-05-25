// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#ifndef SYNCHRONIZATION_H_GUARD
#define SYNCHRONIZATION_H_GUARD

#include <semaphore.h> // sem_t

// Names used in the creation of memory semaphores
#define STR_SEM_MAIN_WALLET_UNREAD      "SEM_MAIN_WALLET_UNREAD"
#define STR_SEM_MAIN_WALLET_FREESPACE   "SEM_MAIN_WALLET_FREESPACE"
#define STR_SEM_MAIN_WALLET_MUTEX       "SEM_MAIN_WALLET_MUTEX"

#define STR_SEM_WALLET_SERVER_UNREAD    "SEM_WALLET_SERVER_UNREAD"
#define STR_SEM_WALLET_SERVER_FREESPACE "SEM_WALLET_SERVER_FREESPACE"
#define STR_SEM_WALLET_SERVER_MUTEX     "SEM_WALLET_SERVER_MUTEX"

#define STR_SEM_SERVER_MAIN_UNREAD      "SEM_SERVER_MAIN_UNREAD"
#define STR_SEM_SERVER_MAIN_FREESPACE   "SEM_SERVER_MAIN_FREESPACE"
#define STR_SEM_SERVER_MAIN_MUTEX       "SEM_SERVER_MAIN_MUTEX"

#define STR_SEM_TERMINATE_MUTEX         "SEM_TERMINATE_MUTEX"         

struct triplet_sems {
    sem_t *unread;      // aka full
    sem_t *free_space;  // aka empty
    sem_t *mutex;
};

// Structure that aggregates information from ALL the necessary semaphores
struct semaphores {
    struct triplet_sems *main_wallet;    // semaphores for access to the buffer between main and wallet
    struct triplet_sems *wallet_server;  // semaphores for access to the buffer between wallet and server
    struct triplet_sems *server_main;    // semaphores for access to the buffer between server and main
    sem_t *terminate_mutex;              
};

/* Function that creates *one* semaphore, initialized to <value> */
sem_t* create_semaphore(char *name, unsigned v);

/* Function to unlink/destroy a semaphore, given its name and pointer */
void destroy_semaphore(char *name, sem_t *sem);

/* Function that creates *all* program semaphores, initializing the free_space semaphores to <v> */
struct semaphores* create_all_semaphores(unsigned v);

/* Print the value of *all* semaphores in <sems> */
void print_all_semaphores(struct semaphores* sems);

/* Function that destroys *all* semaphores in the <sems> structure */
void destroy_all_semaphores(struct semaphores* sems);

/* Generic function that creates 3 semaphores used in the Producer-Consumer logic
1st argument: v - initial value for the free_space semaphore
Remaining arguments: the 3 names to give to the semaphores.
Returns: a pointer to the structure containing 3 semaphores. */
struct triplet_sems* create_triplet_sems(unsigned v, 
    char* freespace_name1, char* unread_name, char* mutex_name);

/* Function that creates the 3 semaphores needed to access the Main-Wallet buffer */
struct triplet_sems* create_main_wallet_sems(unsigned v);

/* Function that creates the 3 semaphores needed to access the Wallet-Server buffer */
struct triplet_sems* create_wallet_server_sems(unsigned v);

/* Function that creates the 3 semaphores needed to access the Server-Main buffer */
struct triplet_sems* create_server_main_sems(unsigned v);

#endif