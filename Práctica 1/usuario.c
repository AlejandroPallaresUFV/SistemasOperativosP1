#include <studio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

struct operacion{
	int tipo;
	int monto;
	int cuentaOR = 0;
	int cuentaDES = 0;
}

void realizar_deposito(){
	operacion op;
	op.tipo = 1;
	op.cuentaDES = Cuenta.numero_cuenta;
	printf("Dinero a depositar:\n");
	scanf("%d", &op.monto);

	write 

}

int main()
{
	pthread_t hilo;
	int opcion;
	while (1){
		printf("1.Deposito\n2.Retiro\n3.Transferencia\n4.Consultar Saldo\n5.salir\n");
		scanf("%d", &opcion);
		switch(opcion){
		case 1: 
			pthread_create(&hilo, NULL, realizar_deposito(), (void*)&);

			break();
		case 2: realizar_retiro();
			break();
		case 3: realizar_transferencia();
			break();
		case 4: consultar_saldo();
			break();
		case 5: exit(0);
		}
	}
	return 0;
}