#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define FILE_NAME "cuentas.txt"

struct Cuenta{
  int numero_cuenta;
  char titular[50];
  float saldo;
  int num_transacciones;
};

int main (){

  int count_cuenta = 1001;
  char op, comp;

  op = 'Y';

  comp = 'N';

  bool flag = true;

  FILE* fp = fopen("cuentas.txt", "a");

  while (flag == true){

    //Inicializa un struct de tipo cuenta

    struct Cuenta c;

    //El count se inicializa en 1000

    c.numero_cuenta = count_cuenta;
    count_cuenta++;

    printf("Escriba el titular:");
    scanf("%50s", c.titular);
    printf("Escriba el saldo incial:");
    scanf("%f", &c.saldo);

    c.num_transacciones = 0;

    fprintf(fp, "%d, %s, %f, %d", c.numero_cuenta, c.titular, c.saldo, c.num_transacciones);



    //En caso de querer añadir más cuentas, se pone "Y"

    printf("Añadir otra cuenta? Y/N");
    scanf("%c", &op);

    if((strcmp(&op,&comp))==0){
      flag = false;
    }

  } 

  fclose(fp);

}