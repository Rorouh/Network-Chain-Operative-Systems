#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      // Para O_CREAT, O_RDWR
#include <sys/mman.h>   // Para mmap, munmap
#include <sys/stat.h>   // Para los modos de permiso
#include "memory.h"

/* ************************************************************
 * FUNCIONES DE MEMORIA DINÁMICA
 * ************************************************************/

/* Reserva una zona de memoria dinámica con el tamaño indicado,
 * la inicializa a 0 y devuelve un puntero a la misma.
 */
// Calloc la reserva y la inicializa a 0, ya que malloc no la inicializa
void* allocate_dynamic_memory(int size) {
    void* ptr = calloc(1, size);
    if (!ptr) {
        perror("Error al asignar memoria dinámica");
        exit(EXIT_FAILURE);
    }
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
 * FUNCIONES PARA ESCRITURA EN BUFFERS COMPARTIDOS
 * ************************************************************/

/* Escribe una transacción en el buffer de memoria compartida
 * entre la Main y las Wallets (ra_buffer). Se utiliza el puntero
 * 'ptrs' como contador de la próxima posición libre.
 * Si el buffer está lleno (índice >= buffer_size), la transacción
 * se pierde.
 */
void write_main_wallets_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    int index = *(buffer->ptrs);
    if (index >= buffer_size) {
        // Buffer lleno, no se escribe la transacción.
        return;
    }
    buffer->buffer[index] = *tx;
    (*(buffer->ptrs))++;
}

/* Escribe una transacción en el buffer circular compartido
 * entre las Wallets y los Servers. Se utiliza la estructura de
 * punteros 'ptrs' (con índices in y out). Si el siguiente índice
 * de escritura coincide con out, el buffer está lleno y se pierde la transacción.
 */
void write_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    int next_in = (buffer->ptrs->in + 1) % buffer_size;
    if (next_in == buffer->ptrs->out) {
        // Buffer lleno, no se escribe la transacción.
        return;
    }
    buffer->buffer[buffer->ptrs->in] = *tx;
    buffer->ptrs->in = next_in;
}

/* Escribe una transacción en el buffer de memoria compartida
 * entre los Servers y la Main (ra_buffer). Funciona de manera similar
 * a write_main_wallets_buffer.
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
 * FUNCIONES PARA LECTURA DE BUFFERS COMPARTIDOS
 * ************************************************************/

/* Lee una transacción del buffer entre la Main y las Wallets (ra_buffer)
 * que esté dirigida a la wallet con id igual a wallet_id.
 * Se recorre el rango 0 a *ptrs-1. Si se encuentra una transacción
 * cuyo src_id coincide y que no haya sido leída (su id distinto de -1),
 * se copia en tx y se marca esa posición como leída (asignando -1 a tx->id).
 * Si no se encuentra, se asigna tx->id = -1.
 */
void read_main_wallets_buffer(struct ra_buffer* buffer, int wallet_id, int buffer_size, struct transaction* tx) {
    int found = 0;
    int count = *(buffer->ptrs);
    for (int i = 0; i < count; i++) {
        if (buffer->buffer[i].id != -1 && buffer->buffer[i].src_id == wallet_id) {
            *tx = buffer->buffer[i];
            // Marcar como leída
            buffer->buffer[i].id = -1;
            found = 1;
            break;
        }
    }
    if (!found) {
        tx->id = -1;
    }
}

/* Lee una transacción del buffer circular entre Wallets y Servers.
 * Si el buffer está vacío (in == out), se asigna tx->id = -1.
 * En caso contrario, se lee la transacción en la posición out y se actualiza out.
 */
void read_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (buffer->ptrs->in == buffer->ptrs->out) {
        tx->id = -1;
        return;
    }
    *tx = buffer->buffer[buffer->ptrs->out];
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
}

/* Lee una transacción del buffer entre Servers y la Main (ra_buffer),
 * buscando la transacción cuyo id sea igual a tx_id. Si se encuentra,
 * se copia en tx y se marca la posición como leída (tx->id = -1).
 * Si no se encuentra, se asigna tx->id = -1.
 */
void read_servers_main_buffer(struct ra_buffer* buffer, int tx_id, int buffer_size, struct transaction* tx) {
    int found = 0;
    int count = *(buffer->ptrs);
    for (int i = 0; i < count; i++) {
        if (buffer->buffer[i].id == tx_id) {
            *tx = buffer->buffer[i];
            // Marcar como leída
            buffer->buffer[i].id = -1;
            found = 1;
            break;
        }
    }
    if (!found) {
        tx->id = -1;
    }
}
