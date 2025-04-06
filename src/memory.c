// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "../inc/memory.h"

/* Função que reserva uma zona de memória dinâmica com o tamanho indicado
 * por size, preenche essa zona de memória com o valor 0, e retorna um 
 * apontador para a mesma.
 */
void* allocate_dynamic_memory(int size){
    //Reservar memoria dinamica
    void *mem = malloc(size);
    if (mem == NULL) {
        printf("Erro na alocação -_- \n");
        return NULL;
    }
    //Rellenar con 0
    memset(mem, 0, size);
    return mem;
}

/* Função que reserva uma zona de memória partilhada com tamanho indicado
 * por size e nome name, preenche essa zona de memória com o valor 0, e 
 * retorna um apontador para a mesma. Pode concatenar o id do utilizador 
 * resultante da função getuid() a name, para tornar o nome único para 
 * a zona de memória.
 */
void* create_shared_memory(char* name, int size) {
    //Reservar memoria compartida
    int fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    int ret = ftruncate(fd, size); 
    if (ret == -1) {
        perror("ftruncate");
        exit(2);
    }
    //Mapeamos memoria
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(3);
    }

    close(fd);

    memset(ptr, 0, size);

    return ptr;
}

/* Liberta uma zona de memória dinâmica previamente alocada.
 */
void deallocate_dynamic_memory(void* ptr){
    //Liberamos memoria
    free(ptr);
}

/* Remove uma zona de memória partilhada previamente criada. 
 */
void destroy_shared_memory(char* name, void* ptr, int size){
    //Desmapeamos
	munmap(ptr, size);
    //Por ultimo, eliminamos
    shm_unlink(name);
}

/* Escreve uma transação no buffer de memória partilhada entre a Main e as carteiras.
 * A transação deve ser escrita numa posição livre do buffer, 
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada e a transação é perdida.
 */
void write_main_wallets_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx){
    for(int i = 0; i < buffer_size; i++){
        //Se la posicion esta libre
        if(buffer->ptrs[i] == 0){
            //Escribimos y ponemos que la posicion esta ocupada
            buffer->ptrs[i] = 1;
            buffer->buffer[i] = *tx;
            return;
        }
    }
}

/* Função que escreve uma transação no buffer de memória partilhada entre as carteiras e os servidores.
 * A transação deve ser escrita numa posição livre do buffer, tendo em conta o tipo de buffer 
 * e as regras de escrita em buffers desse tipo. Se não houver nenhuma posição livre, não escreve nada.
 */
void write_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    //Si el buffer esta lleno
    if((buffer->ptrs->in + 1) % buffer_size ==  buffer->ptrs->out){
        return;
    }
    //Si la posicion esta libre, escribimos
    buffer->buffer[buffer->ptrs->in] = *tx;
    buffer->ptrs->in = (buffer->ptrs->in + 1) % buffer_size;
    
}

/* Função que escreve uma transação no buffer de memória partilhada entre os servidores e a Main, a qual 
 * servirá de comprovativo da execução da transação. A transação deve ser escrita numa posição livre do 
 * buffer, tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo. 
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_servers_main_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    for(int i = 0; i < buffer_size; i++){
        //Si la posicion esta libre, escribimos y ponemos que la posicion esta ocupada
        if(buffer->ptrs[i] == 0){
            buffer->ptrs[i] = 1;
            buffer->buffer[i] = *tx;
            return;
        }
    }
}

/* Função que lê uma transação do buffer entre a Main e as carteiras, se houver alguma disponível para ler 
 * e que seja direcionada a própria carteira que está a tentar ler. A leitura deve ser feita tendo em conta 
 * o tipo de buffer e as regras de leitura em buffers desse tipo. Se não houver nenhuma transação disponível, 
 * afeta tx->id com o valor -1.
 */
void read_main_wallets_buffer(struct ra_buffer* buffer, int wallet_id, int buffer_size, struct transaction* tx){
    for(int i = 0; i < buffer_size; i++){
        //Si la posicion esta ocupada y el id es correcto, leemos y ponemos que la posicion esta libre
        if(buffer->ptrs[i] == 1 && buffer->buffer[i].src_id == wallet_id){
            *tx = buffer->buffer[i];
            buffer->ptrs[i] = 0;
            return;
        }
    }
    //Si no se ha leido ninguna transaccion, ponemos id a -1
    tx->id = -1;
}

/* Função que lê uma transação do buffer entre as carteiras e os servidores, se houver alguma disponível para ler.
 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo. Qualquer
 * servidor pode ler qualquer transação deste buffer. Se não houver nenhuma transação disponível, 
 * afeta tx->id com o valor -1.
 */
void read_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    //Si el buffer esta vacio
    if(buffer->ptrs->in == buffer->ptrs->out){
        tx->id = -1;
        return;
    }
    //Leemos y actualizamos out
    *tx = buffer->buffer[buffer->ptrs->out];
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
}

/* Função que lê uma transação do buffer entre os servidores e a Main, se houver alguma disponível para ler 
 * e que tenha o tx->id igual a tx_id. A leitura deve ser feita tendo em conta o tipo de buffer e as regras 
 * de leitura em buffers desse tipo. Se não houver nenhuma transação disponível, afeta tx->id com o valor -1.
 */
void read_servers_main_buffer(struct ra_buffer* buffer, int tx_id, int buffer_size, struct transaction* tx) {
    for (int i = 0; i < buffer_size; i++) {
        //Si la posicion esta ocupada y el id es correcto, leemos y ponemos que la posicion esta libre
        if (buffer->ptrs[i] == 1 && buffer->buffer[i].id == tx_id) {
            *tx = buffer->buffer[i];    
            buffer->ptrs[i] = 0;        
            return;
        }
    }

    tx->id = -1;
}