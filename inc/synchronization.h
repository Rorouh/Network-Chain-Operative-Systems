#ifndef SYNCHRONIZATION_H_GUARD
#define SYNCHRONIZATION_H_GUARD


#include <semaphore.h> //sem_t

// Nomes usados na criacao dos semaforos
#define STR_SEM_MAIN_WALLET_UNREAD      "SEM_MAIN_WALLET_UNREAD"
#define STR_SEM_MAIN_WALLET_FREESPACE   "SEM_MAIN_WALLET_FREESPACE"
#define STR_SEM_MAIN_WALLET_MUTEX       "SEM_MAIN_WALLET_MUTEX"

#define STR_SEM_WALLET_SERVER_UNREAD    "SEM_WALLET_SERVER_UNREAD"
#define STR_SEM_WALLET_SERVER_FREESPACE "SEM_WALLET_SERVER_FREESPACE"
#define STR_SEM_WALLET_SERVER_MUTEX     "SEM_WALLET_SERVER_MUTEX"

#define STR_SEM_SERVER_MAIN_UNREAD 	    "SEM_SERVER_MAIN_UNREAD"
#define STR_SEM_SERVER_MAIN_FREESPACE 	"SEM_SERVER_MAIN_FREESPACE"
#define STR_SEM_SERVER_MAIN_MUTEX 	    "SEM_SERVER_MAIN_MUTEX"

#define STR_SEM_TERMINATE_MUTEX			"SEM_TERMINATE_MUTEX" 		

struct triplet_sems {
    sem_t *unread; // aka full
    sem_t *free_space; // aka empty
    sem_t *mutex;
};


//estrutura que agrega informação de TODOS os semáforos necessários 
struct semaphores {
	struct triplet_sems *main_wallet;	 // semáforos para acesso ao buffer entre a main e wallet
	struct triplet_sems *wallet_server;  // semáforos para acesso ao buffer entre wallet e server
	struct triplet_sems *server_main; 	// semáforos para acesso ao buffer entre server e main
	sem_t *terminate_mutex;				
};

/* Função que cria *um* semaforo , inicializado a <value> */
sem_t* create_semaphore(char *name, unsigned v);

/* Função para desligar/destruir um semaforo , em funcão do seu nome e pointer */
void destroy_semaphore(char *name, sem_t *sem);

/* Função que cria *todos* os semaforos do programa, inicializando a <v> os semaforos free_space */
struct semaphores* create_all_semaphores(unsigned v);

/* Imprimir o valor de *todos* os semaforos em <sems> */
void print_all_semaphores(struct semaphores* sems);

/* Função que destroi *todos* os semaforos na estrutura <sems> */
void destroy_all_semaphores(struct semaphores* sems);

/* função genérica que cria 3 semaforos usados na lógica Produtor-Consumidor 
1º argumento: v - valor inicial do semaforo free_space
Restantes argumentos: os 3 nomes a dar aos semáforos.
Retorna: um pointer para a estrutura que contem 3 semaforos. */
struct triplet_sems* create_triplet_sems(unsigned v, 
	char*freespace_name1,char*unread_name,char*mutex_name);

/* funcao que cria os 3 semaforos necessários para aceder ao buffer Main-Wallet */
struct triplet_sems* create_main_wallet_sems(unsigned v);

/* funcao que cria os 3 semaforos necessários para aceder ao buffer Wallet-Server */
struct triplet_sems* create_wallet_server_sems(unsigned v);

/* funcao que cria os 3 semaforos necessários para aceder ao buffer Server-Main */
struct triplet_sems* create_server_main_sems(unsigned v);




#endif