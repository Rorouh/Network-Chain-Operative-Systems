// Miguel Angel Lopez Sanchez fc65675
// Alejandro Dominguez fc64447
// Bruno Felisberto fc32435

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "process.h"

int launch_wallet(int wallet_id, struct info_container* info, struct buffers* buffs) {
    int pid = fork();
    if (pid < 0) {
        perror("Error al crear el proceso wallet (fork fallido)");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        //proceso hijo: ejecuta la función execute_wallet.
        int ret = execute_wallet(wallet_id, info, buffs);
        exit(ret);
    }
    //proceso padre:devuelve el pid del proceso hijo.
    return pid;
}

int launch_server(int server_id, struct info_container* info, struct buffers* buffs) {
    int pid = fork();
    if (pid < 0) {
        perror("Error al crear el proceso server (fork fallido)");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        //proceso hijo:ejecuta la función execute_server.
        int ret = execute_server(server_id, info, buffs);
        exit(ret);
    }
    //proceso padre:devuelve el pid del proceso hijo.
    return pid;
}

int wait_process(int process_id) {
    int status;
    if (waitpid(process_id, &status, 0) == -1) {
        perror("Error en waitpid");
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;  //el proceso no termino correctamente
    }
}
