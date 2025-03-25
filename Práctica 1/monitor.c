#include <studio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

//Funcion para realizar una op sobr eel semaforo
void operar_semaforo(int semid, int op){
	struct sembuf operacion;
	operacion.sem_nun = 0; //Indice semaforo
	operacion.sem_op = op; // -1 (wait/P), +1 (signal/V)
	opreracion.sem_flg = 0;
	semop(semid, &operacion, 1); //aplicar op
}


int main (){

	key_t key = ftok ("./misemaforo",1);

	if(key == (key_t) -1){
		perror("Error al generar key");
		exit(1);
	}

	int semid = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); //Crear semaforo
	if (semid == -1){
		perror("Error al crear el semaforo");
		exit(1);
	}

	//inicializar semaforo a 1 (recurso disponble)

	semctl (semid, 0, SETVAL, 1); //semforo de acceso a zona critica
	semctl (semid, 1, SETVAL, 3); //Semaforo de limite de scritura
	semctl (semid, 2, SETVAL, 0); //semaforo de pila vacia
}