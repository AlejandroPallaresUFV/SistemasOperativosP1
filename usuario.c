#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
 
// Estructura de cuenta bancaria
typedef struct {
    int numero_cuenta;
    char titular[50];
    float saldo;
    int num_transacciones;
} Cuenta;

// Estructura para comunicación con banco.c
typedef struct {
    int numero_cuenta;
    char tipo[20];
    float monto;
} Operacion;

// Globales
sem_t *semaforo;
char archivo_cuentas[] = "cuentas.dat";
int cuenta_actual;

// Función para buscar una cuenta en el archivo
long buscar_posicion(FILE *f, Cuenta *cuenta) {
    rewind(f);
    while (fread(cuenta, sizeof(Cuenta), 1, f)) {
        if (cuenta->numero_cuenta == cuenta_actual) {
            return ftell(f) - sizeof(Cuenta);
        }
    }
    return -1;
}

// Función de hilo para depósito, retiro o transferencia
void *procesar_operacion(void *arg) {
    Operacion *op = (Operacion *)arg;
    FILE *f = fopen(archivo_cuentas, "rb+");
    if (!f) {
        perror("Error abriendo cuentas.dat");
        return NULL;
    }

    sem_wait(semaforo);  // Bloquear acceso

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

    if (strcmp(op->tipo, "deposito") == 0) {
        cuenta.saldo += op->monto;
    } else if (strcmp(op->tipo, "retiro") == 0) {
        if (cuenta.saldo >= op->monto) {
            cuenta.saldo -= op->monto;
        } else {
            printf("Saldo insuficiente.\n");
            sem_post(semaforo);
            fclose(f);
            return NULL;
        }
    } else if (strcmp(op->tipo, "transferencia") == 0) {
        int destino;
        printf("Cuenta destino: ");
        scanf("%d", &destino);

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

            struct msgbuf {
                long tipo;
                char texto[100];
            };

            key_t key = ftok("monitor.c", 65);
            int cola_id = msgget(key, 0666);
            if (cola_id != -1) {
                struct msgbuf mensaje;
                mensaje.tipo = 1;
                snprintf(mensaje.texto, sizeof(mensaje.texto), "%d,%s,%.2f", op->numero_cuenta, op->tipo, op->monto);
                
                msgsnd(cola_id, &mensaje, sizeof(mensaje.texto), 0);
            }

            Operacion notificacion = { destino, "recibido", op->monto };
            write(STDOUT_FILENO, &notificacion, sizeof(Operacion));

            sem_post(semaforo);
            fclose(f);
            return NULL;
        } else {
            printf("Saldo insuficiente para transferencia.\n");
        }
    }

    // Depósito o retiro: actualizar cuenta y notificar
    fseek(f, pos, SEEK_SET);
    fwrite(&cuenta, sizeof(Cuenta), 1, f);
    write(STDOUT_FILENO, op, sizeof(Operacion));
    printf("Operación realizada con éxito.\n");

    sem_post(semaforo);
    fclose(f);
    return NULL;
}

// Consultar saldo
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

// Menú principal
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <ID_usuario>\n", argv[0]);
        return 1;
    }

    printf("Introduce tu número de cuenta: ");
    scanf("%d", &cuenta_actual);

    semaforo = sem_open("/cuentas_sem", 0);
    if (semaforo == SEM_FAILED) {
        perror("Error al abrir semáforo");
        return 1;
    }

    int opcion;
    do {
        printf("\n--- Menú Usuario ---\n");
        printf("1. Depósito\n");
        printf("2. Retiro\n");
        printf("3. Transferencia\n");
        printf("4. Consultar saldo\n");
        printf("5. Salir\n");
        printf("Opción: ");
        scanf("%d", &opcion);

        if (opcion >= 1 && opcion <= 3) {
            Operacion op;
            op.numero_cuenta = cuenta_actual;

            printf("Monto: ");
            scanf("%f", &op.monto);

            if (opcion == 1) strcpy(op.tipo, "deposito");
            else if (opcion == 2) strcpy(op.tipo, "retiro");
            else if (opcion == 3) strcpy(op.tipo, "transferencia");

            pthread_t hilo;
            pthread_create(&hilo, NULL, procesar_operacion, &op);
            pthread_join(hilo, NULL);

        } else if (opcion == 4) {
            consultar_saldo();
        }

    } while (opcion != 5);

    sem_close(semaforo);

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