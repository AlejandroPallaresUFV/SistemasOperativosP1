#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define NOMBRE_SEM "/cuentas_sem"
#define MAX_USUARIOS 4

int main() {
    int pipe_monitor[2];
    pid_t pid_monitor;
    sem_t *sem;
    sem = sem_open(NOMBRE_SEM, O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    if (pipe(pipe_monitor) == -1) {
        perror("pipe monitor");
        exit(1);
    }

    pid_monitor = fork();

    switch (pid_monitor) {
        case -1:
            perror("Error al hacer fork para monitor");
            exit(1);

        case 0: {
            close(pipe_monitor[0]);
            char fd_monitor_write[10];
            sprintf(fd_monitor_write, "%d", pipe_monitor[1]);
            execl("./monitor", "./monitor", fd_monitor_write, NULL);
            perror("Error al lanzar monitor");
            exit(1);
        }
    }

    // Lanzar múltiples usuarios con xterm
    for (int i = 0; i < MAX_USUARIOS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char id_str[10];
            snprintf(id_str, sizeof(id_str), "%d", 1001 + i);
            execlp("xterm", "xterm", "-e", "./usuario", id_str, NULL);
            perror("Error lanzando xterm con usuario");
            exit(1);
        }
    }

    close(pipe_monitor[1]);

    char alerta[256];
    int leidos;

    while ((leidos = read(pipe_monitor[0], alerta, sizeof(alerta) - 1)) > 0) {
        alerta[leidos] = '\0';
        printf("Alerta de MONITOR: %s", alerta);
    }

    close(pipe_monitor[0]);

    for (int i = 0; i < MAX_USUARIOS + 1; i++) {
        wait(NULL);
    }

    sem_unlink(NOMBRE_SEM);

    return 0;
}
