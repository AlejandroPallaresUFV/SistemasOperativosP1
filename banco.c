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

int main() {
    int pipe_monitor[2];
    pid_t pid_monitor, pid_usuario;
    sem_t *sem;
    leer_configuracion();

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

    pid_usuario = fork();

    switch (pid_usuario) {
        case -1:
            perror("Error al hacer fork para usuario");
            exit(1);

        case 0:
            execl("./usuario", "./usuario", "1001", NULL);
            perror("Error al lanzar usuario");
            exit(1);
    }

    close(pipe_monitor[1]);

    char alerta[256];
    int leidos;

    while ((leidos = read(pipe_monitor[0], alerta, sizeof(alerta) - 1)) > 0) {
        alerta[leidos] = '\0';
        printf("Alerta de MONITOR: %s", alerta);
    }

    close(pipe_monitor[0]);

    wait(NULL);
    wait(NULL);

    sem_unlink(NOMBRE_SEM);

    return 0;
}
