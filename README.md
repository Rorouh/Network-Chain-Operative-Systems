# SOchain

**Autores**:

* Miguel Ángel López Sánchez (fc65675)
* Alejandro Domínguez (fc64447)
* Bruno Felisberto (fc32435)

---

## Tabla de Contenidos

1. [Descripción del Proyecto](#descripción-del-proyecto)
2. [Requisitos y Dependencias](#requisitos-y-dependencias)
3. [Estructura del Repositorio](#estructura-del-repositorio)
4. [Ficheros Principales](#ficheros-principales)

   * [1. `main.c`](#1-mainc)
   * [2. `memory.c` / `memory.h`](#2-memoryc--memoryh)
   * [3. `synchronization.c` / `synchronization.h`](#3-synchronizationc--synchronizationh)
   * [4. `process.c` / `process.h`](#4-processc--processh)
   * [5. `wallet.c` / `wallet.h`](#5-walletc--walleth)
   * [6. `server.c` / `server.h`](#6-serverc--serverh)
   * [7. `ctime.c` / `ctime.h`](#7-ctimec--ctimeh)
   * [8. `clog.c` / `clog.h`](#8-clogc--clogh)
   * [9. `csettings.c` / `csettings.h`](#9-csettingsc--csettingsh)
   * [10. `csignals.c` / `csignals.h`](#10-csignalsc--csignalsh)
   * [11. `cstats.c` / `cstats.h`](#11-cstatsc--cstatsh)
5. [Ficheros de Configuración](#ficheros-de-configuración)

   * [`args.txt`](#argstxt)
   * [`settings.txt`](#settingstxt)
6. [Compilación](#compilación)
7. [Ejecución](#ejecución)
8. [Interfaz de Usuario / Comandos](#interfaz-de-usuario--comandos)
9. [Funcionamiento Interno](#funcionamiento-interno)

   * [Modelo de Procesos](#modelo-de-procesos)
   * [Memoria Compartida](#memoria-compartida)
   * [Comunicación y Sincronización](#comunicación-y-sincronización)
   * [Flujo de una Transacción](#flujo-de-una-transacción)
10. [Manejo de Señales](#manejo-de-señales)
11. [Registro de Operaciones (Logging)](#registro-de-operaciones-logging)
12. [Pruebas y Casos de Uso](#pruebas-y-casos-de-uso)
13. [Conclusión](#conclusión)

---

## Descripción del Proyecto

**SOchain** es una aplicación de ejemplo que simula una cadena de bloques simplificada (“blockchain-like”) en un entorno multiproceso, utilizando memoria compartida y semáforos para la sincronización. Cada transacción creada por el usuario es “firmada” por una cartera (wallet) y luego procesada por uno de varios servidores. El sistema finalmente genera un “recibo” (receipt) de la transacción, que puede ser consultado desde la interfaz principal.

El objetivo era poner en práctica conceptos de Sistemas Operativos:

* **Procesos múltiples** (fork)
* **Memoria compartida** (`shm_open`, `mmap`)
* **Semáforos POSIX** (`sem_open`, `sem_wait`, `sem_post`, `sem_trywait`)
* **Buffers circulares y de acceso aleatorio**
* **Comunicación entre procesos (Producer–Consumer)**
* **Manejo de señales**
* **Registro (logging) y estadísticas en tiempo real**

---

## Requisitos y Dependencias

* GNU/Linux
* GCC (o compatible)
* Librerías POSIX (`pthread` para semáforos, `mman` para memoria compartida—vienen con la libc de sistemas UNIX)
* `make` (opcional, si se incluye un Makefile)

---

## Estructura del Repositorio

```text
SOchain/
├── bin/                     # Ejecutables compilados
│   └── SOchain
│
├── inc/                     # Headers (.h)
│   ├── csettings.h
│   ├── csignals.h
│   ├── cstats.h
│   ├── clog.h
│   ├── ctime.h
│   ├── main.h
│   ├── memory.h
│   ├── process.h
│   ├── server.h
│   ├── synchronization.h
│   ├── wallet.h
│   └── ... (otros headers)
│
├── src/                     # Fuentes (.c)
│   ├── main.c
│   ├── memory.c
│   ├── process.c
│   ├── server.c
│   ├── wallet.c
│   ├── synchronization.c
│   ├── csettings.c
│   ├── csignals.c
│   ├── cstats.c
│   ├── clog.c
│   ├── ctime.c
│   └── ... (otros .c)
│
├── args.txt                 # Archivo de argumentos (inicialización)
├── settings.txt             # Archivo de configuración (wallets, servidores, buffers, etc.)
├── Makefile                 # Opcional: para compilar todo con `make`
├── README.md                # Este documento
└── logs/                    # Carpeta donde se almacenan los logs (si aplica)
    └── operations.log
```

---

## Ficheros Principales

### 1. `main.c`

* **Propósito**: Función `main()` y ciclo de interacción con el usuario.
* **Contenido Principal**:

  * Lectura de `args.txt` y `settings.txt`
  * Creación de semáforos (llamando a `create_all_semaphores`)
  * Reserva de memoria dinámica y compartida
  * Lanzamiento de procesos “wallet” y “server” mediante fork
  * Bucle `user_interaction()` que espera comandos:

    * `bal`, `trx`, `rcp`, `stat`, `help`, `end`
  * Cuando se digita `end`, señaliza `terminate=1`, despierta procesos, espera finalización, imprime estadísticas finales, destruye recursos compartidos y semáforos, sale.

#### Funciones Clave en `main.c`:

* `main_args(...)`
* `create_dynamic_memory_structs(...)`
* `create_shared_memory_structs(...)`
* `create_processes(...)`
* `user_interaction(...)`
* `print_balance(...)`
* `create_transaction(...)`
* `receive_receipt(...)`
* `print_stat(...)`
* `help()`
* `end_execution(...)`
* `wakeup_processes(...)`
* `wait_processes(...)`

---

### 2. `memory.c` / `memory.h`

* **Propósito**: Gestión de memoria dinámica y memoria compartida, además de operaciones de escritura/lectura en buffers.

#### `memory.h` (NO MODIFICAR en evaluación)

* Definición de identificadores `ID_SHM_…`
* Estructuras:

  * `struct pointers` (para buffer circular)
  * `struct circ_buffer` (pointers + buffer array)
  * `struct ra_buffer` (punteros + buffer array)
  * `struct transaction` (ID, src\_id, dest\_id, amount, wallet\_signature, server\_signature, timestamps)
  * `struct buffers` (tres buffers: Main→Wallets, Wallets→Servers, Servers→Main)
* Prototipos de funciones:

  * `allocate_dynamic_memory(size)`
  * `create_shared_memory(name, size)`
  * `deallocate_dynamic_memory(ptr)`
  * `destroy_shared_memory(name, ptr, size)`
  * `write_main_wallets_buffer(...)`
  * `write_wallets_servers_buffer(...)`
  * `write_servers_main_buffer(...)`
  * `read_main_wallets_buffer(...)`
  * `read_wallets_servers_buffer(...)`
  * `read_servers_main_buffer(...)`

#### `memory.c`

* **`allocate_dynamic_memory(int size)`**:

  * Reserva `malloc(size)`, inicializa a cero (`memset`).
* **`create_shared_memory(char* name, int size)`**:

  * `shm_open` para crear/open por nombre, `ftruncate` para fijar tamaño, `mmap` en modo *shared*, cierra fd, `memset` a 0.
  * Retorna puntero a la zona compartida.
* **`destroy_shared_memory(char* name, void* ptr, int size)`**:

  * `munmap(ptr, size)` + `shm_unlink(name)`.
* **Funciones de buffer**:

  * `write_main_wallets_buffer(...)`: Recorre `ptrs[]` buscando un slot libre (`ptrs[i]==0`), marca `1`, copia transacción.
  * `write_wallets_servers_buffer(...)`: Buffer circular. Comprueba `(in+1)%n == out` → lleno. Si no, copia en `buffer[in]` y avanza `in`.
  * `write_servers_main_buffer(...)`: Igual que `write_main_wallets_buffer` (RA-buffer).
  * `read_main_wallets_buffer(...)`: Recorre buscando `ptrs[i]==1 && buffer[i].src_id == wallet_id`. Si lo halla, copia a `tx`, marca `ptrs[i]=0`. Si ninguno, `tx->id=-1`.
  * `read_wallets_servers_buffer(...)`: Si `in == out`, vació → `tx->id=-1`; else lee `buffer[out]`, incrementa `out`.
  * `read_servers_main_buffer(...)`: Recorre `ptrs[i]==1 && buffer[i].id == tx_id`; si coincide, copia y marca `ptrs[i]=0`; si no hallado, `tx->id=-1`.

---

### 3. `synchronization.c` / `synchronization.h`

* **Propósito**: Creación y destrucción de semáforos POSIX, estructuras de semáforos agrupados para el patrón productor–consumidor.

#### `synchronization.h`

* Define macros de nombres (`STR_SEM_…`) para cada semáforo.
* Estructuras:

  * `struct triplet_sems { sem_t *unread, *free_space, *mutex; }`
  * `struct semaphores { struct triplet_sems *main_wallet, *wallet_server, *server_main; sem_t *terminate_mutex; }`
* Prototipos:

  * `create_semaphore(name, v)`
  * `destroy_semaphore(name, sem_t*)`
  * `create_all_semaphores(v)`
  * `print_all_semaphores(...)`
  * `destroy_all_semaphores(...)`
  * `create_triplet_sems(v, ..., ..., ...)`
  * `create_main_wallet_sems(v)`, `create_wallet_server_sems(v)`, `create_server_main_sems(v)`

#### `synchronization.c`

* **`create_semaphore(char* name, unsigned v)`**:

  * `sem_unlink(name)` para eliminar posible residuo, `sem_open(name, O_CREAT | O_EXCL, 0644, v)`. Si falla, muestra `perror` y `exit(6)`.
* **`destroy_semaphore(char* name, sem_t* sem)`**:

  * `sem_close(sem)`, `sem_unlink(name)`.
* **`create_triplet_sems(v, freespace_name, unread_name, mutex_name)`**:

  * Reserva `malloc(sizeof(triplet_sems))`.
  * `free_space = create_semaphore(freespace_name, v)`
  * `unread     = create_semaphore(unread_name, 0)`
  * `mutex      = create_semaphore(mutex_name, 1)`
* **`create_all_semaphores(v)`**:

  * Combina las tres tripletas para:

    1. Main→Wallets
    2. Wallets→Servers
    3. Servers→Main
  * Además crea `terminate_mutex = create_semaphore("SEM_TERMINATE_MUTEX", 1)`.
* **`print_all_semaphores(...)`**:

  * Muestra valores actuales de cada semáforo con `sem_getvalue`.
* **`destroy_all_semaphores(...)`**:

  * Cierra y unlink de cada semáforo en el orden inverso a la creación (free\_space, unread, mutex), libera memoria de `struct triplet_sems`, y por último libera `semaphores`.

---

### 4. `process.c` / `process.h`

* **Propósito**: Envolver la lógica de `fork`/`waitpid` para lanzar procesos “wallet” y “server”.

#### `process.h`

```c
#ifndef PROCESS_H_GUARD
#define PROCESS_H_GUARD

#include "main.h"

int launch_wallet(int wallet_id, struct info_container* info, struct buffers* buffs);
int launch_server(int server_id, struct info_container* info, struct buffers* buffs);
int wait_process(int process_id);

#endif
```

#### `process.c`

* **`launch_wallet(wallet_id, info, buffs)`**:

  * `fork()`, en hijo llama `execute_wallet(wallet_id, info, buffs)` y `exit(ret)`.
  * El padre devuelve el PID.
* **`launch_server(server_id, info, buffs)`**:

  * Igual que `launch_wallet`, pero llama `execute_server(server_id, info, buffs)`.
* **`wait_process(process_id)`**:

  * `waitpid(process_id, &status, 0)`, si terminó normalmente, devuelve `WEXITSTATUS(status)`, si no, `-1`.

---

### 5. `wallet.c` / `wallet.h`

* **Propósito**: Implementa el proceso “wallet”—firma transacciones que le corresponden (src\_id) y las envía al buffer de servidores.

#### `wallet.h`

```c
#ifndef WALLET_H_GUARD
#define WALLET_H_GUARD

#include "main.h"
#include "memory.h"

int execute_wallet(int wallet_id, struct info_container* info, struct buffers* buffs);
void wallet_receive_transaction(struct transaction* tx, int wallet_id, struct info_container* info, struct buffers* buffs);
void wallet_process_transaction(struct transaction* tx, int wallet_id, struct info_container* info);
void wallet_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs);

#endif
```

#### `wallet.c`

* **`execute_wallet(wallet_id, info, buffs)`**:

  * Bucle infinito hasta que `*(info->terminate)==1`.
  * Llama `wallet_receive_transaction(&tx, wallet_id, info, buffs)`, que hace:

    * `sem_wait(main_wallet->unread)` (espera que haya algo para leer)
    * `sem_wait(main_wallet->mutex)`
    * `read_main_wallets_buffer(buffs->buff_main_wallets, wallet_id, ..., &tx)`
    * `sem_post(main_wallet->mutex)`
    * `sem_post(main_wallet->free_space)`
    * Si `tx.id == -1`, `usleep(100000)` → pasa a siguiente iteración.
  * Si `tx.src_id == wallet_id`, llama `wallet_process_transaction(&tx, wallet_id, info)`, que simplemente:

    * `tx->wallet_signature = wallet_id`
    * `info->wallets_stats[wallet_id]++`
  * Luego `save_time(...)` y `wallet_send_transaction(&tx, info, buffs)` que hace:

    * `sem_wait(wallet_server->free_space)`
    * `sem_wait(wallet_server->mutex)`
    * `write_wallets_servers_buffer(buffs->buff_wallets_servers, ..., &tx)`
    * `sem_post(wallet_server->mutex)`
    * `sem_post(wallet_server->unread)`
  * Reinicia `tx.id = -1` y repite.
  * Cuando termina, retorna el número de transacciones firmadas (`transacciones_firmadas`).

---

### 6. `server.c` / `server.h`

* **Propósito**: Implementa el proceso “server”—lee transacciones firmadas, las valida (fondos, firmas), actualiza saldos y envía recibos a la main.

#### `server.h`

```c
#ifndef SERVER_H_GUARD
#define SERVER_H_GUARD

#include "main.h"
#include "memory.h"

int execute_server(int server_id, struct info_container* info, struct buffers* buffs);
void server_receive_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs);
void server_process_transaction(struct transaction* tx, int server_id, struct info_container* info);
void server_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs);

#endif
```

#### `server.c`

* **`execute_server(server_id, info, buffs)`**:

  * Bucle infinito hasta `*(info->terminate)==1`.
  * Llama `sem_wait(wallet_server->unread)` y `sem_wait(wallet_server->mutex)`
  * `server_receive_transaction(&tx, info, buffs)`, que hace `read_wallets_servers_buffer(...)`
  * `sem_post(wallet_server->mutex)` y `sem_post(wallet_server->free_space)`
  * Si `tx.id == -1`, `usleep(100000)` y continuar.
  * Llama `server_process_transaction(&tx, server_id, info)`, que:

    * Comprueba `0 ≤ src_id < n_wallets` y `0 ≤ dest_id < n_wallets`.
    * Verifica `wallet_signature == src_id`.
    * Verifica `balances[src_id] ≥ amount`.
    * Si válido: `balances[src_id] -= amount; balances[dest_id] += amount;`
    * Firma: `tx->server_signature = server_id + 1`.
    * Incrementa `info->servers_stats[server_id]++`.
  * Si `tx.server_signature != 0`, entonces:

    * `sem_wait(server_main->free_space)`
    * `sem_wait(server_main->mutex)`
    * `server_send_transaction(&tx, info, buffs)`
    * `sem_post(server_main->mutex)`
    * `sem_post(server_main->unread)`
    * Aumenta `transacciones_procesadas`.
  * Reinicia `tx.id = -1`.
  * Al salir del bucle (terminate), retorna `transacciones_procesadas`.

---

### 7. `ctime.c` / `ctime.h`

* **Propósito**: Gestionar marcas de tiempo para cada etapa de la transacción (Main, Wallet, Server).
* **Estructura `struct timestamps`** (dentro de `memory.h`):

  ```c
  struct timestamps {
      struct timespec main;
      struct timespec wallets;
      struct timespec servers;
  };
  ```
* **Funciones**:

  * `void save_time(struct timespec* ts)`

    * Obtiene la hora actual (`clock_gettime(CLOCK_REALTIME, ts)`) e imprime un log a `clog`.
  * Permite calcular retrasos o estadísticas de latencia.

---

### 8. `clog.c` / `clog.h`

* **Propósito**: Logging de operaciones con marca de tiempo al inicio del proyecto (fallback).
* **Funciones Clave**:

  * `void save_operation(const char* op, const char* log_filename)`

    * Abre (o crea) archivo `log_filename` en modo appended
    * Obtiene hora actual, formatea la línea:

      ```
      YYYYMMDD HH:MM:SS.mmm <op>
      ```
    * Escribe en disco y cierra.
  * `void write_statistics(struct info_container* info)`

    * Crea un fichero `stats.txt` (o similar) con información final de WS, SS, etc.

---

### 9. `csettings.c` / `csettings.h`

* **Propósito**: Leer y parsear los archivos de texto `args.txt` y `settings.txt` para rellenar `info_container`.

#### `csettings.h`

```c
#ifndef CSETTINGS_H_GUARD
#define CSETTINGS_H_GUARD

#include "main.h"

void read_args(const char *filename, struct info_container *info);
void read_settings(const char *filename, struct info_container *info);

#endif
```

#### `csettings.c`

* **`read_args(filename, info)`**:

  * Abre `args.txt`, lee línea con:

    ```
    init_balance=<float>
    n_wallets=<int>
    n_servers=<int>
    buffers_size=<int>
    max_txs=<int>
    log_filename=<string>
    stats_filename=<string>
    ```
  * Guarda en `info->init_balance`, `info->n_wallets`, `info->n_servers`, `info->buffers_size`, `info->max_txs`, `info->log_filename`, `info->stats_filename`.
  * Valida valores; si alguno inválido, `fprintf(stderr, ...)` y `exit(1)`.
* **`read_settings(filename, info)`**:

  * Abre `settings.txt`, lee formato:

    ```
    wallet_proc_schedule=<float>  # intervalo ms entre lecturas en wallets
    server_proc_schedule=<float>  # intervalo ms entre lecturas en servidores
    max_slots=<int>               # tamaño de los buffers circulares/RA
    … (otras configuraciones específicas)
    ```
  * Rellena valores en `info->wallet_delay`, `info->server_delay`, etc.

*(Los campos exactos dependen de la convención interna; este ejemplo ilustra la idea.)*

---

### 10. `csignals.c` / `csignals.h`

* **Propósito**: Manejo de señales (Ctrl+C) para terminar limpiamente.

#### `csignals.h`

```c
#ifndef CSIGNALS_H_GUARD
#define CSIGNALS_H_GUARD

#include "main.h"
void setup_ctrlC_signal(struct info_container* info, struct buffers* buffs);
void setup_ctrlC_signal_parent(void);

#endif
```

#### `csignals.c`

* **`setup_ctrlC_signal(info, buffs)`**:

  * Llame a `signal(SIGINT, handler_wallet_or_server)`.
  * En el handler (código estático), si recibe SIGINT desde proceso hijo (wallet/server), copia a `*(info->terminate)=1`, libera semáforos para despertar bloqueos, sale `exit(...)`.
* **`setup_ctrlC_signal_parent()`**:

  * `signal(SIGINT, handler_main)` para que cuando el padre reciba Ctrl+C, también llame a `end_execution(...)`.

---

### 11. `cstats.c` / `cstats.h`

* **Propósito**: Calcular métricas y estadísticas en tiempo real, aparte de la impresión final.

#### `cstats.h`

```c
#ifndef CSTATS_H_GUARD
#define CSTATS_H_GUARD

#include "main.h"
void update_stats_on_transaction(int tx_id, struct info_container* info);
void print_stats_to_file(const char* filename, struct info_container* info);

#endif
```

#### `cstats.c`

* **`update_stats_on_transaction(tx_id, info)`**:

  * Actualiza contadores internos (e.g., latencia, contadores de transacciones por wallet/servidor).
* **`print_stats_to_file(filename, info)`**:

  * Al escribir estadísticas finales, vuelca toda la info de latencias promedio, contadores, etc.

*(Estos archivos pueden extenderse según las necesidades de métricas.)*

---

## Ficheros de Configuración

### `args.txt`

Este archivo define parámetros básicos para iniciar el sistema (lectura en `read_args()`).

Ejemplo:

```
init_balance=100.0
n_wallets=3
n_servers=2
buffers_size=10
max_txs=100
log_filename=logs/operations.log
stats_filename=logs/stats.txt
```

* `init_balance` (float)   : Saldo inicial de cada wallet.
* `n_wallets` (int)        : Número de procesos “wallet” (carteras).
* `n_servers` (int)        : Número de procesos “server” (servidores).
* `buffers_size` (int)     : Tamaño de los buffers compartidos (RA y circular).
* `max_txs` (int)          : Límite máximo de transacciones que el usuario puede crear.
* `log_filename` (string)  : Ruta del fichero donde se registran operaciones (`bal`, `trx`, `rcp`, `stat`, `help`, `end`).
* `stats_filename` (string): Ruta del fichero donde se guardan las estadísticas finales.

### `settings.txt`

Define parámetros internos de scheduling y delays (lectura en `read_settings()`).

Ejemplo (valores ilustrativos):

```
wallet_delay_ms=100
server_delay_ms=100
```

* `wallet_delay_ms` (int): Milisegundos de espera cuando una wallet no encuentre transacción válida.
* `server_delay_ms` (int): Milisegundos de espera cuando un servidor no encuentre transacción válida.

*(Se pueden añadir más configuraciones según el diseño.)*

---

## Compilación

1. Asegúrate de tener instalado gcc, make y las librerías POSIX.

2. Desde la raíz del proyecto, si existe un `Makefile`, simplemente ejecuta:

   ```bash
   make
   ```

   Esto generará el ejecutable `bin/SOchain`.

3. Si no tienes `Makefile`, compila manualmente (suponiendo que todos los `.c` están en `src/` y cabeceras en `inc/`):

   ```bash
   gcc -std=c11 -pthread -lrt -o bin/SOchain \
       src/*.c
   ```

   * `-pthread` para semáforos POSIX.
   * `-lrt` para funciones de reloj de alta resolución (`clock_gettime`).

---

## Ejecución

Desde la carpeta raíz del proyecto:

```bash
./bin/SOchain args.txt settings.txt
```

* **Primer parámetro**: ruta a `args.txt`.
* **Segundo parámetro**: ruta a `settings.txt`.

Si todo está bien, verás en pantalla la lista de comandos disponibles. Luego podrás interactuar con el sistema.

---

## Interfaz de Usuario / Comandos

Al iniciar, se imprime:

```
Comandos disponíveis:
  bal   - Mostrar saldo da carteira
  trx   - Criar uma nova transação
  rcp   - Obter um recibo de uma transação
  stat  - Imprimir estatísticas atuais do sistema
  help  - Mostrar esta mensagem de ajuda
  end   - Terminar a execução da SOchain
```

Luego, cada línea comienza con `SOchain> `. Los comandos se comportan así:

1. **`bal`**

   * Muestra el saldo actual de una wallet.
   * **Flujo**:

     * El usuario ingresa `bal`.
     * El programa pide: `Introduza o ID da carteira:`
     * El usuario teclea un entero válido `id` (0 ≤ id < `n_wallets`).
     * Se imprime `Saldo da carteira <id>: <saldo> SOT`.
     * Se registra la operación en el log: `bal <id>`.

2. **`trx`**

   * Crea una nueva transacción (órdenes encoladas en Main→Wallets).
   * **Flujo**:

     * El usuario ingresa `trx`.
     * El programa pide: `Insira src_id, dest_id e valor:`
     * El usuario ingresa: `<src_id> <dest_id> <amount>`
     * Se validan:

       * `0 ≤ src_id < n_wallets`
       * `0 ≤ dest_id < n_wallets`
       * `amount > 0 && amount ≤ balances[src_id]`
       * `src_id != dest_id`
     * Si inválido → imprime `Dados de transação inválidos.` y retorna.
     * Si válido:

       * Construye `struct transaction tx` con `id = tx_counter++`, `src_id`, `dest_id`, `amount`, `wallet_signature = 0`, `server_signature = 0`.
       * Guarda timestamp en `tx.change_time.main`.
       * **Semáforos**:

         1. `sem_wait(main_wallet->free_space)` (espera slot libre)
         2. `sem_wait(main_wallet->mutex)` (entra sección crítica)
         3. `write_main_wallets_buffer(buffs->buff_main_wallets, buffers_size, &tx)`
         4. `sem_post(main_wallet->mutex)`
         5. `sem_post(main_wallet->unread)` (despierta wallets bloqueadas)
       * Imprime `Transação criada (id <tx.id>): <src_id> -> <dest_id>, amount = <amount>`.
       * Registra operación en log: `trx <src_id> <dest_id> <amount>`.

3. **`rcp`**

   * Obtiene el recibo (receipt) de una transacción procesada (servidores→Main).

   * **Flujo**:

     * El usuario ingresa `rcp`.
     * El programa pide: `Insira o ID da transação para obter o comprovativo:`
     * El usuario ingresa un entero `tx_id`.
     * **Semáforos**:

       1. `sem_trywait(server_main->unread)`

          * Si errno == `EAGAIN`: no hay recibos pendientes → imprime `Nenhum comprovativo encontrado para a transação <tx_id>.` y retorna.
          * Si falla otro error → muestra `perror("sem_trywait")`, retorna.
       2. `sem_wait(server_main->mutex)` (entra sección crítica)
       3. Llama `read_servers_main_buffer(buffs->buff_servers_main, tx_id, buffers_size, &tx)`

          * Recorre slots: busca `ptrs[i] == 1 && buffer[i].id == tx_id`.
          * Si lo halla: copia transacción `tx`, marca `ptrs[i]=0`.
          * Si no halla: `tx.id = -1`.
       4. `sem_post(server_main->mutex)`
       5. Si `tx.id == -1`:

          * No era el recibo buscado → `sem_post(server_main->unread)` (devolver “permiso”)
          * Imprime `Nenhum comprovativo encontrado para a transação <tx_id>.`
       6. Si `tx.id != -1`:

          * ¡Era el correcto! → `sem_post(server_main->free_space)` (liberar espacio)
          * Imprime:

            ```
            Recibo da transação <tx_id>:
              src_id: <tx.src_id>, dest_id: <tx.dest_id>, amount: <tx.amount>
              wallet_signature: <tx.wallet_signature>, server_signature: <tx.server_signature>
            ```
          * Registra `op = snprintf("rcp %d", tx_id)` en el log.

   * **Importante**: Si el retorno de `read_servers_main_buffer` arroja `tx.id == -1`, ello no significa que nunca existirá ese `tx_id`; puede estar ocupando otra posición en el buffer. Por eso devolvemos `unread` para no “gastarlo”.

4. **`stat`**

   * Imprime estadísticas en tiempo real:

     * Configuración inicial (`init_balance`, `n_wallets`, `n_servers`, `buffers_size`, `max_txs`, `tx_counter`, `terminate`).
     * PIDs de wallets y servidores.
     * Saldos actuales de cada wallet.
     * Transacciones firmadas por cada wallet (`info->wallets_stats[i]`).
     * Transacciones procesadas por cada servidor (`info->servers_stats[i]`).
   * Registra `stat` en el log.

5. **`help`**

   * Muestra la lista de todos los comandos disponibles.
   * Registra `help` en el log.

6. **`end`**

   * Señaliza `*(info->terminate) = 1`.
   * Llama a `wakeup_processes(info)` (para liberar semáforos y “despertar” wallets/servidores bloqueados).
   * Llama a `wait_processes(info)` (espera a que terminen todos los procesos hijos).
   * Llama a `write_final_statistics(info)` (imprime estadísticas finales en pantalla).
   * Llama a `write_statistics(info)` (vogará estadísticas en disco).
   * Llama a `save_operation("end", log_filename)`.
   * Destruye toda la memoria compartida (`destroy_shared_memory_structs`) y dinámica (`destroy_dynamic_memory_structs`).
   * Destruye semáforos (`destroy_all_semaphores(info->sems)`).
   * Libera `info` y `buffs` (que fueron `malloc` en `main`).
   * `exit(1)`.

---

## Funcionamiento Interno

### Modelo de Procesos

1. **Proceso Padre (`main`)**

   * Se encarga de:

     * Leer configuración (`args.txt`, `settings.txt`).
     * Crear semáforos y buffers compartidos.
     * Lanzar `n_wallets` procesos hijos “wallet” y `n_servers` procesos hijos “server”.
     * Operar como “interfaz de usuario”: aceptar comandos `bal`, `trx`, `rcp`, etc.
     * Al recibir `end` o señal SIGINT, notifica `terminate=1`, espera a que hijos terminen y limpia todo.

2. **Procesos “Wallet” (n\_wallets)**

   * Cada wallet (ID = 0, 1, …, `n_wallets-1`) hace:

     1. Espera a que exista una transacción para él en `buff_main_wallets`:

        * `sem_wait(main_wallet->unread)`
        * `sem_wait(main_wallet->mutex)`
        * `read_main_wallets_buffer(..., wallet_id, ...)`
        * `sem_post(main_wallet->mutex)`, `sem_post(main_wallet->free_space)`
     2. Si `tx.id == -1`: espera 100 ms y repite.
     3. Si `tx.src_id == wallet_id`: firma la transacción:

        * `tx.wallet_signature = wallet_id`
        * `info->wallets_stats[wallet_id]++`
        * `save_time(&tx.change_time.wallets)`
        * Envía a `buff_wallets_servers`:

          * `sem_wait(wallet_server->free_space)`
          * `sem_wait(wallet_server->mutex)`
          * `write_wallets_servers_buffer(...)`
          * `sem_post(wallet_server->mutex)`, `sem_post(wallet_server->unread)`
     4. Reinicia `tx.id = -1` y vuelve al paso 1.

3. **Procesos “Server” (n\_servers)**

   * Cada servidor (ID = 0, 1, …, `n_servers-1`) hace:

     1. Espera a que exista una transacción firmada en `buff_wallets_servers`:

        * `sem_wait(wallet_server->unread)`
        * `sem_wait(wallet_server->mutex)`
        * `server_receive_transaction(&tx, ...)`
        * `sem_post(wallet_server->mutex)`, `sem_post(wallet_server->free_space)`
     2. Si `tx.id == -1`: espera 100 ms y repite.
     3. Valida la transacción:

        * Chequea `0 ≤ tx.src_id < n_wallets`, `0 ≤ tx.dest_id < n_wallets`.
        * Verifica `tx.wallet_signature == tx.src_id`.
        * Verifica `balances[tx.src_id] ≥ tx.amount`.
        * Si falla cualquiera → descarta (`tx.id` queda `-1`) y repite paso 1.
     4. Si es válida:

        * Actualiza saldos:

          ```c
          info->balances[tx.src_id] -= tx.amount;
          info->balances[tx.dest_id] += tx.amount;
          ```
        * Firma de servidor: `tx.server_signature = server_id + 1`
        * `info->servers_stats[server_id]++`
        * `save_time(&tx.change_time.servers)`
        * Escribe recibo en `buff_servers_main`:

          * `sem_wait(server_main->free_space)`
          * `sem_wait(server_main->mutex)`
          * `write_servers_main_buffer(...)`
          * `sem_post(server_main->mutex)`, `sem_post(server_main->unread)`
        * `transacciones_procesadas++`
     5. Reinicia `tx.id = -1` y repite.

---

### Memoria Compartida

Utilizamos tres zonas de memoria compartida para buffers:

1. **Main → Wallets** (`struct ra_buffer *buff_main_wallets`)

   * `ptrs` es un arreglo de `int[buffers_size]`, inicialmente todos 0.
   * `buffer` es un arreglo de `struct transaction[buffers_size]`.
   * `ptrs[i] == 0` → slot vacío, `ptrs[i] == 1` → slot ocupado (transacción pendiente de ser leída).

2. **Wallets → Servers** (`struct circ_buffer *buff_wallets_servers`)

   * `ptrs` es `struct pointers` con dos índices:

     * `in`: siguiente posición de escritura (inicialmente 0).
     * `out`: siguiente posición de lectura (inicialmente 0).
   * Buffer circular de tamaño `buffers_size`.
   * Vacío si `in == out`. Lleno si `(in + 1) % buffers_size == out`.

3. **Servers → Main** (`struct ra_buffer *buff_servers_main`)

   * Igual que Main→Wallets: buffer de acceso aleatorio, en el que los servidores escriben recibos y el padre (main) lee por `tx_id`.

---

### Comunicación y Sincronización

Para cada buffer compartido disponemos de tres semáforos agrupados en un `triplet_sems`:

* **`free_space`**

  * Inicializado con `v = buffers_size` (para `main_wallet` y `server_main`) o `v = buffers_size - 1` (para circular `wallet_server`) según convención.
  * Cuenta cuántos slots libres hay para escribir.

* **`unread`**

  * Inicializado en 0. Cuenta cuántas entradas “listas para leer” hay en el buffer.

* **`mutex`**

  * Inicializado en 1. Mutex de acceso exclusivo a la sección crítica del buffer.

Además, existe un semáforo adicional **`terminate_mutex`** para sincronizar el apagado de procesos (no se usa en cada buffer, sino solo para asegurarse de que la señal de terminación se propaga correctamente).

#### Patrón Prod–Cons (Producer–Consumer)

* **Productor = Main** produce transacciones → `buff_main_wallets`:

  * `sem_wait(free_space)`
  * `sem_wait(mutex)`
  * Escribe
  * `sem_post(mutex)`, `sem_post(unread)`

* **Consumidor = Wallets** en paralelo:

  * `sem_wait(unread)`
  * `sem_wait(mutex)`
  * Lee
  * `sem_post(mutex)`, `sem_post(free_space)`

Luego **Wallets** pasan a **Ser­vers** en un buffer circular con los semáforos análogos.

Por último, **Servers** pasan recibos a **Main** en `buff_servers_main`, con semáforos análogos.

---

### Flujo de una Transacción (Resumen)

1. **Usuario**: `trx <src_id> <dest_id> <amount>`
2. **Main**: crea `tx` y lo encola en `buff_main_wallets`.
3. **Wallet `<src_id>`**: desencola, firma, lo encola en `buff_wallets_servers`.
4. **Servidor `<k>`**: desencola, valida, firma y actualiza saldos, encola recibo en `buff_servers_main`.
5. **Usuario**: `rcp <tx_id>` → Main desencola el recibo de `buff_servers_main` y lo muestra.

---

## Manejo de Señales

### 1. `SIGINT` (Ctrl+C)

* **Procesos hijos (wallets y servers)**:

  * `setup_ctrlC_signal(info, buffs)` instala un handler.
  * Cuando reciben SIGINT (usuario presiona Ctrl+C en la consola del padre), el handler hace `*(info->terminate)=1`, libera semáforos con `wakeup_processes(info)` para despertar bloqueos y sale.

* **Proceso padre (`main`)**:

  * `setup_ctrlC_signal_parent()` instala handler para SIGINT que simplemente invoca `end_execution(info, buffs)`, dejando que el sistema se termine limpiamente.

Esta estrategia garantiza que **todas** las rutinas bloqueadas en `sem_wait(...)` quedan despertadas para terminar.

---

## Registro de Operaciones (Logging)

Todas las operaciones del usuario se registran en un archivo que se especifica en `args.txt` como `log_filename`:

* Cada vez que se ingresa un comando válido:

  * `bal <id>`
  * `trx <src_id> <dest_id> <amount>`
  * `rcp <tx_id>`
  * `stat`
  * `help`
  * `end`

aparece una línea nueva en ese log con formato:

```
YYYYMMDD HH:MM:SS.mmm <comando>
```

Ejemplo:

```
20250525 03:15:30.758 trx 0 1 5.00
20250525 03:16:18.671 bal 1
20250525 03:16:21.474 bal 0
20250525 03:16:24.139 end
```

Además, al final de la ejecución, se puede volcar información complementaria (latencias, conteos, etc.) en un `stats_filename`.

---

## Pruebas y Casos de Uso

A continuación se presentan ejemplos típicos de interacción y cómo verificar que el sistema funciona correctamente.

### 1. Primera ejecución

```bash
$ ./bin/SOchain args.txt settings.txt

Comandos disponíveis:
  bal   - Mostrar saldo da carteira
  trx   - Criar uma nova transação
  rcp   - Obter un recibo de una transação
  stat  - Imprimir estatísticas atuais do sistema
  help  - Mostrar esta mensagem de ajuda
  end   - Terminar a execução da SOchain

SOchain> bal
Introduza o ID da carteira: 0
Saldo da carteira 0: 100.00 SOT

SOchain> trx 0 1 10
Insira src_id, dest_id e valor: Transação criada (id 0): 0 -> 1, amount = 10.00
```

* Se todo está bien, la wallet 0 firmará, el servidor 1 procesará, y finalmente aparecerá el recibo disponible.

### 2. Obtener recibo

```bash
SOchain> rcp 0
Insira o ID da transação para obter o comprovativo: Recibo da transação 0:
  src_id: 0, dest_id: 1, amount: 10.00
  wallet_signature: 0, server_signature: 1
```

* El sistema localiza la transacción en `buff_servers_main`, la elimina, libera un slot y la muestra.
* Si se intenta otra vez `rcp 0`, ya no estará disponible → `Nenhum comprovativo encontrado para a transação 0.`

### 3. Transacción inválida o repetida

```bash
SOchain> trx 0 0 5
Insira src_id, dest_id e valor: Dados de transação inválidos.  # src_id == dest_id

SOchain> trx 0 1 500
Insira src_id, dest_id e valor: Dados de transação inválidos.  # fondos insuficientes
```

### 4. Límite de transacciones

Si se alcanza `max_txs`, al ingresar `trx` aparecerá:

```
Número máximo de transações alcançadas.
```

### 5. Estadísticas en tiempo real

```bash
SOchain> stat

--- Estatísticas Atuais ---
Saldo inicial: 100.00
Número de carteiras: 3
Número de servidores: 2
Estatísticas Atuais ...: 10
Máximo de transações: 100
Transações criadas: 2
Flag terminate: 0
PIDs das carteiras:
  Carteira 0: 12345
  Carteira 1: 12346
  Carteira 2: 12347
PIDs dos servidores:
  Servidor 1: 12348
  Servidor 2: 12349
Saldos actuais das carteiras:
  Carteira 0: 90.00 SOT
  Carteira 1: 110.00 SOT
  Carteira 2: 100.00 SOT
Transações de cada carteira:
  Carteira 0: 1
  Carteira 1: 0
  Carteira 2: 0
Transações de cada servidor:
  Servidor 1: 1
  Servidor 2: 0
```

* Confirma que los índices internos (buffers\_size, max\_txs, etc.) son correctos.
* Verifica el número de transacciones creadas y firmadas.

### 6. Señales (Ctrl+C)

* Si el usuario presiona **Ctrl+C** en cualquier momento:

  * El padre (main) detecta SIGINT, invoca `end_execution`.
  * Se imprimen estadísticas finales, se limpian semáforos y memoria compartida, y el programa termina sin “zombies” ni bloqueos.
  * Todos los hijos reciben `terminate=1` y mueren ordenadamente.

---

## Conclusión

El proyecto **SOchain** ejemplifica:

1. Cómo orquestar múltiples procesos (fork) en C.
2. Uso de **memoria compartida** (`shm_open`, `mmap`) para compartir datos (buffers) entre procesos.
3. Uso de **semáforos POSIX** (`sem_open`, `sem_wait`, `sem_post`, `sem_trywait`, `sem_unlink`) para sincronizar productores–consumidores en tres etapas (Main→Wallets, Wallets→Servers, Servers→Main).
4. Gestión de **buffers**: tanto circulares (para Wallets→Servers) como de acceso aleatorio (Main→Wallets y Servers→Main).
5. Mecanismos de **manejo de señales** para apagar el sistema ordenadamente.
6. Mecanismos de **logging** y **estadísticas** para auditoría y trazabilidad.

Gracias a su arquitectura modular (separación en archivos `memory.c`, `synchronization.c`, `wallet.c`, `server.c`, etc.), resulta fácil implementar extensiones:

* Agregar validaciones adicionales en el proceso de servidor.
* Incluir métricas de latencia usando las marcas de tiempo en `ctime.c`.
* Ajustar `settings.txt` para regular el tiempo de espera de wallets y servidores.
* Ampliar `cstats.c` para generar gráficos o estadísticas avanzadas.
