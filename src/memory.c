// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "../inc/memory.h"


void* allocate_dynamic_memory(int size){
    void *mem = malloc(size);
    if (mem == NULL) {
        printf("Erro na alocação -_- \n");
        return NULL;
    }
    memset(mem, 0, size);
    return mem;
}

void* create_shared_memory(char* name, int size) {
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

    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(3);
    }

    close(fd);

    memset(ptr, 0, size);

    return ptr;
}
void deallocate_dynamic_memory(void* mem){
    free(mem);
}

void destroy_shared_memory(char* name, void* mem, int size){
	munmap(mem, size);
    shm_unlink(name);
}

void write_main_wallets_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx){
    for(int i = 0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 0){
            buffer->ptrs[i] = 1;
            buffer->buffer[i] = *tx;
            return;
        }
    }
}

void write_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    if((buffer->ptrs->in + 1) % buffer_size ==  buffer->ptrs->out){
        return;
    }

    buffer->buffer[in] = *tx;
    buffer->ptrs->in = (in + 1) % buffer_size;
    
}

void write_servers_main_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    for(int i = 0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 0){
            buffer->ptrs[i] = 1;
            buffer->buffer[i] = *tx;
            return;
        }
    }
}

void read_main_wallets_buffer(struct ra_buffer* buffer, int wallet_id, int buffer_size, struct transaction* tx){
    for(int i = 0; i < buffer_size; i++){
        if(buffer->ptrs[i] == 1 && buffer->buffer[i].dest_id == wallet_id){
            *tx = buffer->buffer[i];
            buffer->ptrs[i] = 0;
            return;
        }
    }
    tx->id = -1;
}

void read_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    if(buffer->ptrs->in == buffer->ptrs->out){
        tx->id = -1;
        return;
    }
    *tx = buffer->buffer[buffer->ptrs->out];
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
}

void read_servers_main_buffer(struct ra_buffer* buffer, int tx_id, int buffer_size, struct transaction* tx) {
    for (int i = 0; i < buffer_size; i++) {
        if (buffer->ptrs[i] == 1 && buffer->buffer[i].id == tx_id) {
            *tx = buffer->buffer[i];    
            buffer->ptrs[i] = 0;        
            return;
        }
    }

    tx->id = -1;
}
