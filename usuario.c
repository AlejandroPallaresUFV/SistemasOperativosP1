#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "estructuras.h"

sem_t *semaforo;
char archivo_cuentas[] = "cuentas.dat";
char archivo_transacciones[] = "transacciones.log";
int cuenta_actual;

long buscar_posicion(FILE *f, Cuenta *cuenta) {
    rewind(f);
    while (fread(cuenta, sizeof(Cuenta), 1, f)) {
        if (cuenta->numero_cuenta == cuenta_actual) {
            return ftell(f) - sizeof(Cuenta);
        }
    }
    return -1;
}

void *procesar_operacion(void *arg) {
    DatosOperacion *op = (DatosOperacion *)arg;
    FILE *f = fopen(archivo_cuentas, "rb+");
    if (!f) {
        perror("Error abriendo cuentas.dat");
        return NULL;
    }

    sem_wait(semaforo);

    Cuenta cuenta;
    long pos = buscar_posicion(f, &cuenta);
    if (pos == -1) {
        printf("Cuenta no encontrada.\n");
        sem_post(semaforo);
        fclose(f);
        return NULL;
    }

    fseek(f, pos, SEEK_SET);
    fread(&cuenta, sizeof(Cuenta), 1, f);

    if (op->tipo_operacion == 1) { // deposito
        cuenta.saldo += op->monto;

        FILE *t = fopen(archivo_transacciones, "a");
        fprintf(t,"Cuenta destino: %d | Monto ingresado: %.2f\n", cuenta_actual, op->monto);
        fclose(t);

    } else if (op->tipo_operacion == 2) { // retiro
        if (cuenta.saldo >= op->monto) {
            cuenta.saldo -= op->monto;

            FILE *t = fopen(archivo_transacciones, "a");
            fprintf(t,"Cuenta origen: %d | Monto retirado: %.2f\n", cuenta_actual, op->monto);
            fclose(t);


        } else {
            printf("Saldo insuficiente.\n");
            sem_post(semaforo);
            fclose(f);
            return NULL;
        }
    } else if (op->tipo_operacion == 3) { // transferencia
        int destino;
        printf("Cuenta destino: ");
        scanf("%d", &destino);
        op->cuenta_destino = destino;

        Cuenta cuenta_dest;
        long pos_dest = -1;
        rewind(f);
        while (fread(&cuenta_dest, sizeof(Cuenta), 1, f)) {
            if (cuenta_dest.numero_cuenta == destino) {
                pos_dest = ftell(f) - sizeof(Cuenta);
                break;
            }
        }

        if (pos_dest == -1) {
            printf("Cuenta destino no encontrada.\n");
            sem_post(semaforo);
            fclose(f);
            return NULL;
        }

        if (cuenta.saldo >= op->monto) {
            cuenta.saldo -= op->monto;
            fseek(f, pos, SEEK_SET);
            fwrite(&cuenta, sizeof(Cuenta), 1, f);

            cuenta_dest.saldo += op->monto;
            fseek(f, pos_dest, SEEK_SET);
            fwrite(&cuenta_dest, sizeof(Cuenta), 1, f);

            printf("Transferencia realizada correctamente.\n");

            FILE *t = fopen(archivo_transacciones, "a");
            fprintf(t,"Cuenta origen: %d | Cuenta destino: %d | Monto transferido: %.2f\n", cuenta_actual, cuenta_dest.numero_cuenta, op->monto);
            fclose(t);

        } else {
            printf("Saldo insuficiente para transferencia.\n");
            sem_post(semaforo);
            fclose(f);
            return NULL;
        }
    }

    fseek(f, pos, SEEK_SET);
    fwrite(&cuenta, sizeof(Cuenta), 1, f);
    printf("Operación realizada con éxito.\n");

    // Envío al monitor (para cualquier operación)
    key_t key = ftok("monitor.c", 65);
    int cola_id = msgget(key, 0666);
    if (cola_id != -1) {
        struct msgbuf mensaje;
        mensaje.mtype = 1;
        snprintf(mensaje.mtext, sizeof(mensaje.mtext), "%d,%s,%.2f,%d",
                 op->numero_cuenta,
                 op->tipo_operacion == 1 ? "deposito" : (op->tipo_operacion == 2 ? "retiro" : "transferencia"),
                 op->monto,
                 op->cuenta_destino);
        msgsnd(cola_id, &mensaje, sizeof(mensaje.mtext), 0);
    }

    sem_post(semaforo);
    fclose(f);
    return NULL;
}

void consultar_saldo() {
    sem_wait(semaforo);

    FILE *f = fopen(archivo_cuentas, "rb");
    if (!f) {
        perror("Error abriendo cuentas.dat");
        sem_post(semaforo);
        return;
    }

    Cuenta cuenta;
    long pos = buscar_posicion(f, &cuenta);
    if (pos != -1) {
        fseek(f, pos, SEEK_SET);
        fread(&cuenta, sizeof(Cuenta), 1, f);
        printf("Titular: %s | Saldo actual: %.2f\n", cuenta.titular, cuenta.saldo);
    } else {
        printf("Cuenta no encontrada.\n");
    }

    fclose(f);
    sem_post(semaforo);
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <numero_cuenta>\n", argv[0]);
        return 1;
    }

    cuenta_actual = atoi(argv[1]);
    printf("🧾 Accediendo a la cuenta: %d\n", cuenta_actual);

    semaforo = sem_open("/cuentas_sem", 0);
    if (semaforo == SEM_FAILED) {
        perror("Error al abrir semáforo");
        return 1;
    }

    int opcion;
    do {
        printf("\n--- Menú Usuario [%d] ---\n", cuenta_actual);
        printf("1. Depósito\n");
        printf("2. Retiro\n");
        printf("3. Transferencia\n");
        printf("4. Consultar saldo\n");
        printf("5. Salir\n");
        printf("Opción: ");
        scanf("%d", &opcion);

        if (opcion >= 1 && opcion <= 3) {
            DatosOperacion op;
            op.numero_cuenta = cuenta_actual;
            op.write_fd = STDOUT_FILENO;

            printf("Monto: ");
            scanf("%f", &op.monto);

            if (opcion == 1) {
                op.tipo_operacion = 1; // depósito
            } else if (opcion == 2) {
                op.tipo_operacion = 2; // retiro
            } else if (opcion == 3) {
                op.tipo_operacion = 3; // transferencia
                printf("Cuenta destino: ");
                scanf("%d", &op.cuenta_destino);
            }

            pthread_t hilo;
            pthread_create(&hilo, NULL, procesar_operacion, &op);
            pthread_join(hilo, NULL);

        } else if (opcion == 4) {
            consultar_saldo();
        }

    } while (opcion != 5);


    sem_close(semaforo);

    // Enviar mensaje de FIN al monitor al salir
    key_t key = ftok("monitor.c", 65);
    int cola_id = msgget(key, 0666);
    if (cola_id != -1) {
        struct msgbuf {
            long tipo;
            char texto[100];
        } fin;
        fin.tipo = 1;
        strcpy(fin.texto, "FIN");
        msgsnd(cola_id, (void *)&fin, sizeof(fin.texto), 0);
    }

    return 0;
}
