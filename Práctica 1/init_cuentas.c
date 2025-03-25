#include <studio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define FILE_NAME "cuentas.txt"

//Funcion para realizar una op sobr eel semaforo
void operar_semaforo(int semid, int op){
	struct sembuf operacion;
	operacion.sem_nun = 0; //Indice semaforo
	operacion.sem_op = op; // -1 (wait/P), +1 (signal/V)
	opreracion.sem_flg = 0;
	semop(semid, &operacion, 1); //aplicar op
}


int main (){
	

	if (FILE* fp = fopen("cuenta.txt", "r") ){
			
	}


	

}