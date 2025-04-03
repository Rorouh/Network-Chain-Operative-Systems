#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      // Para O_CREAT, O_RDWR
#include <sys/mman.h>   // Para mmap, munmap
#include <sys/stat.h>   // Para los modos de permiso
#include "../inc/memory.h"

/* ************************************************************
 * FUNCIONES DE MEMORIA DINÁMICA
 * ************************************************************/

/* Reserva una zona de memoria dinámica con el tamaño indicado,
 * usando malloc y luego inicializando a 0 mediante memset.
 */
void* allocate_dynamic_memory(int size) {
    void* ptr = malloc(size);
    if (!ptr) {
        perror("Error al asignar memoria dinámica");
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, size);
    return ptr;
}

/* Libera una zona de memoria dinámica previamente asignada.
 */
void deallocate_dynamic_memory(void* ptr) {
    free(ptr);
}

/* ************************************************************
 * FUNCIONES DE MEMORIA COMPARTIDA
 * ************************************************************/

/* Reserva una zona de memoria compartida con el tamaño indicado
 * por size y nombre name. Para hacerlo único se concatena el UID.
 * La zona se inicializa a 0 y se devuelve un puntero a la misma.
 */
void* create_shared_memory(char* name, int size) {
    char unique_name[256];
    snprintf(unique_name, sizeof(unique_name), "%s_%d", name, getuid());
    
    int fd = shm_open(unique_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error al crear la memoria compartida");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fd, size) == -1) {
        perror("Error al ajustar el tamaño de la memoria compartida");
        exit(EXIT_FAILURE);
    }
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Error al mapear la memoria compartida");
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, size);
    close(fd);
    return ptr;
}

/* Elimina una zona de memoria compartida previamente creada.
 * Realiza munmap y shm_unlink utilizando un nombre único.
 */
void destroy_shared_memory(char* name, void* ptr, int size) {
    char unique_name[256];
    snprintf(unique_name, sizeof(unique_name), "%s_%d", name, getuid());
    
    if (munmap(ptr, size) == -1) {
        perror("Error al desmapear la memoria compartida");
    }
    if (shm_unlink(unique_name) == -1) {
        perror("Error al eliminar la memoria compartida");
    }
}

/* ************************************************************
 * FUNCIONES AUXILIARES PARA GESTIÓN DE BUFFERS COMPARTIDOS
 * Estas funciones son llamadas desde main.c, por lo que deben definirse.
 * ************************************************************/

/* Crea la memoria compartida para los buffers de comunicación.
 * Se crean tres buffers:
 *  - Entre Main y Wallets (ra_buffer)
 *  - Circular entre Wallets y Servers (circ_buffer)
 *  - Entre Servers y Main (ra_buffer)
 */
void create_shared_memory_for_buffers(struct buffers* buffs, int buffer_size) {
    // Buffer entre Main y Wallets (ra_buffer)
    buffs->buff_main_wallets = allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_main_wallets->ptrs = (int*) create_shared_memory(ID_SHM_MAIN_WALLETS_PTR, sizeof(int));
    buffs->buff_main_wallets->buffer = (struct transaction*) create_shared_memory(ID_SHM_MAIN_WALLETS_BUFFER, sizeof(struct transaction) * buffer_size);
    *(buffs->buff_main_wallets->ptrs) = 0;
    
    // Buffer circular entre Wallets y Servers (circ_buffer)
    buffs->buff_wallets_servers = allocate_dynamic_memory(sizeof(struct circ_buffer));
    buffs->buff_wallets_servers->ptrs = (struct pointers*) create_shared_memory(ID_SHM_WALLETS_SERVERS_PTR, sizeof(struct pointers));
    buffs->buff_wallets_servers->buffer = (struct transaction*) create_shared_memory(ID_SHM_WALLETS_SERVERS_BUFFER, sizeof(struct transaction) * buffer_size);
    buffs->buff_wallets_servers->ptrs->in = 0;
    buffs->buff_wallets_servers->ptrs->out = 0;
    
    // Buffer entre Servers y Main (ra_buffer)
    buffs->buff_servers_main = allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_servers_main->ptrs = (int*) create_shared_memory(ID_SHM_SERVERS_MAIN_PTR, sizeof(int));
    buffs->buff_servers_main->buffer = (struct transaction*) create_shared_memory(ID_SHM_SERVERS_MAIN_BUFFER, sizeof(struct transaction) * buffer_size);
    *(buffs->buff_servers_main->ptrs) = 0;
}

/* Libera la memoria compartida creada para los buffers.
 */
void destroy_shared_memory_for_buffers(struct buffers* buffs, int buffer_size) {
    destroy_shared_memory(ID_SHM_MAIN_WALLETS_PTR, buffs->buff_main_wallets->ptrs, sizeof(int));
    destroy_shared_memory(ID_SHM_MAIN_WALLETS_BUFFER, buffs->buff_main_wallets->buffer, sizeof(struct transaction) * buffer_size);
    
    destroy_shared_memory(ID_SHM_WALLETS_SERVERS_PTR, (void*)buffs->buff_wallets_servers->ptrs, sizeof(struct pointers));
    destroy_shared_memory(ID_SHM_WALLETS_SERVERS_BUFFER, buffs->buff_wallets_servers->buffer, sizeof(struct transaction) * buffer_size);
    
    destroy_shared_memory(ID_SHM_SERVERS_MAIN_PTR, buffs->buff_servers_main->ptrs, sizeof(int));
    destroy_shared_memory(ID_SHM_SERVERS_MAIN_BUFFER, buffs->buff_servers_main->buffer, sizeof(struct transaction) * buffer_size);
}

/* ************************************************************
 * FUNCIONES PARA ESCRITURA EN BUFFERS COMPARTIDOS
 * ************************************************************/

/* Escribe una transacción en el buffer entre Main y Wallets (ra_buffer).
 * Se utiliza el puntero "ptrs" como contador de la posición libre.
 * Si el buffer está lleno (índice >= buffer_size), la transacción se descarta.
 */
void write_main_wallets_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    int index = *(buffer->ptrs);
    if (index >= buffer_size) {
        return;
    }
    buffer->buffer[index] = *tx;
    (*(buffer->ptrs))++;
}

/* Escribe una transacción en el buffer circular entre Wallets y Servers.
 * Calcula el siguiente índice (in + 1 mod buffer_size) y, si coincide con out (buffer lleno),
 * descarta la transacción; de lo contrario, la escribe y actualiza el índice in.
 */
void write_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    int next_in = (buffer->ptrs->in + 1) % buffer_size;
    if (next_in == buffer->ptrs->out) {
        return;
    }
    buffer->buffer[buffer->ptrs->in] = *tx;
    buffer->ptrs->in = next_in;
}

/* Escribe una transacción en el buffer entre Servers y Main (ra_buffer).
 */
void write_servers_main_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    int index = *(buffer->ptrs);
    if (index >= buffer_size) {
        return;
    }
    buffer->buffer[index] = *tx;
    (*(buffer->ptrs))++;
}

/* ************************************************************
 * FUNCIONES PARA LECTURA EN BUFFERS COMPARTIDOS
 * ************************************************************/

/* Lee una transacción del buffer entre Main y Wallets (ra_buffer)
 * destinada a la wallet con id igual a wallet_id.
 * Recorre las posiciones de 0 a (*ptrs - 1); si encuentra una transacción
 * no leída (id != -1) y cuyo src_id coincide, la copia en tx y la marca como leída (asigna -1 al id).
 * Si no se encuentra, asigna tx->id = -1.
 */
void read_main_wallets_buffer(struct ra_buffer* buffer, int wallet_id, int buffer_size, struct transaction* tx) {
    int found = 0;
    int count = *(buffer->ptrs);
    for (int i = 0; i < count; i++) {
        if (buffer->buffer[i].id != -1 && buffer->buffer[i].src_id == wallet_id) {
            *tx = buffer->buffer[i];
            buffer->buffer[i].id = -1;  // Marcar como leída
            found = 1;
            break;
        }
    }
    if (!found) {
        tx->id = -1;
    }
}

/* Lee una transacción del buffer circular entre Wallets y Servers.
 * Si el buffer está vacío (in == out), asigna tx->id = -1.
 * De lo contrario, lee la transacción en la posición "out" y actualiza ese índice.
 */
void read_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (buffer->ptrs->in == buffer->ptrs->out) {
        tx->id = -1;
        return;
    }
    *tx = buffer->buffer[buffer->ptrs->out];
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
}

/* Lee una transacción del buffer entre Servers y Main (ra_buffer),
 * buscando aquella con id igual a tx_id. Si se encuentra, la copia en tx
 * y la marca como leída (asigna -1 al id); de lo contrario, asigna tx->id = -1.
 */
void read_servers_main_buffer(struct ra_buffer* buffer, int tx_id, int buffer_size, struct transaction* tx) {
    int found = 0;
    int count = *(buffer->ptrs);
    for (int i = 0; i < count; i++) {
        if (buffer->buffer[i].id == tx_id) {
            *tx = buffer->buffer[i];
            buffer->buffer[i].id = -1;
            found = 1;
            break;
        }
    }
    if (!found) {
        tx->id = -1;
    }
}
