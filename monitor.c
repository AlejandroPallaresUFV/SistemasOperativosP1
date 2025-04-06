#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>

float LIMITE_RETIRO = 1000.0;
float LIMITE_TRANSFERENCIA = 5000.0;
int UMBRAL_RETIROS = 3;
int UMBRAL_TRANSFERENCIAS = 5;

struct msgbuf {
    long tipo;
    char texto[100];
};

typedef struct {
    int cuenta;
    int retiros_consecutivos;
    float ultimo_monto;
    int uso_concurrente;
    char ultima_operacion[20];
    char ultimo_destino[20];
} EstadoCuenta;

EstadoCuenta cuentas[100];
int num_cuentas = 0;

EstadoCuenta *get_estado(int cuenta) {
    for (int i = 0; i < num_cuentas; i++) {
        if (cuentas[i].cuenta == cuenta) return &cuentas[i];
    }
    cuentas[num_cuentas].cuenta = cuenta;
    cuentas[num_cuentas].retiros_consecutivos = 0;
    cuentas[num_cuentas].uso_concurrente = 0;
    num_cuentas++;
    return &cuentas[num_cuentas - 1];
}

void leer_configuracion() {
    FILE *f = fopen("config.txt", "r");
    if (!f) {
        perror("Error abriendo config.txt");
        return;
    }

    char linea[100];
    while (fgets(linea, sizeof(linea), f)) {
        if (strncmp(linea, "LIMITE_RETIRO=", 14) == 0) {
            sscanf(linea + 14, "%f", &LIMITE_RETIRO);
        } else if (strncmp(linea, "LIMITE_TRANSFERENCIA=", 22) == 0) {
            sscanf(linea + 22, "%f", &LIMITE_TRANSFERENCIA);
        } else if (strncmp(linea, "UMBRAL_RETIROS=", 16) == 0) {
            sscanf(linea + 16, "%d", &UMBRAL_RETIROS);
        } else if (strncmp(linea, "UMBRAL_TRANSFERENCIAS=", 24) == 0) {
            sscanf(linea + 24, "%d", &UMBRAL_TRANSFERENCIAS);
        }
    }

    fclose(f);
}


int main(int argc, char *argv[]) {

   leer_configuracion();

    if (argc < 2) {
        printf("Uso: %s <fd_alertas>\n", argv[0]);
        return 1;
    }

    int pipe_fd = atoi(argv[1]);

    key_t key = ftok("monitor.c", 65);
    int cola_id = msgget(key, 0666 | IPC_CREAT);
    if (cola_id == -1) {
        perror("msgget");
        return 1;
    }


    while (1) {
        struct msgbuf mensaje;
        if (msgrcv(cola_id, &mensaje, sizeof(mensaje.texto), 0, 0) == -1) {
            perror("msgrcv");
            continue;
        }
        if (strcmp(mensaje.texto, "FIN") == 0) {
        printf("Finalizando\n");
        break;
    }

        int cuenta;
        char tipo[20], destino[20] = "";
        float monto;

        int campos = sscanf(mensaje.texto, "%d,%[^,],%f,%s", &cuenta, tipo, &monto, destino);
        EstadoCuenta *estado = get_estado(cuenta);

        if (strcmp(tipo, "retiro") == 0 && monto >= LIMITE_RETIRO) {
            if (strcmp(estado->ultima_operacion, "retiro") == 0)
                estado->retiros_consecutivos++;
            else
                estado->retiros_consecutivos = 1;

            if (estado->retiros_consecutivos >= 2) {
                char alerta[100];
                snprintf(alerta, sizeof(alerta),
                         "ALERTA: Retiros sospechosos consecutivos en cuenta %d\n", cuenta);
                write(pipe_fd, alerta, strlen(alerta));
            }
        } else {
            estado->retiros_consecutivos = 0;
        }

        if (strcmp(tipo, "transferencia") == 0 && campos == 4) {
            if (strcmp(estado->ultima_operacion, "transferencia") == 0 &&
                strcmp(estado->ultimo_destino, destino) == 0) {

                char alerta[100];
                snprintf(alerta, sizeof(alerta),
                         "ALERTA: Transferencias repetidas desde %d a %s\n", cuenta, destino);
                write(pipe_fd, alerta, strlen(alerta));
            }
            strcpy(estado->ultimo_destino, destino);
        }

        estado->uso_concurrente++;
        if (estado->uso_concurrente > 1) {
            char alerta[100];
            snprintf(alerta, sizeof(alerta),
                     "ALERTA: Uso simultáneo detectado en cuenta %d\n", cuenta);
            write(pipe_fd, alerta, strlen(alerta));
        }

        strcpy(estado->ultima_operacion, tipo);
        estado->uso_concurrente--;

        printf("[MONITOR] %s\n", mensaje.texto);
    }

    return 0;
}