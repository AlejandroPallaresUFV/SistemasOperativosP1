#include <studio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

struct Cuenta{
  int numero_cuenta;
  char titular[50];
  float saldo;
  int num_transacciones;
}

int main()
{

  sem_t*semaforo=sem_open("/cuentas_sem", O_CREAT, 0644, 1);

  while(){


    printf("Usuario:\n");
    scanf("%d", &usuario);
    printf("Contraseña:\n");
    scanf("%d", &contrasenia);

    if (usuario == )

    pid_t pid = fork();



    if (pid == 0){
      ejecutar_menu_usuario(pipe_padre_hijo);
    }
    else{

    }
    
  }
  

  return 0;
}