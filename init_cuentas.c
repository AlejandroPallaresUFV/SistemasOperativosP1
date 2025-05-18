#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estructuras.h"

// Función que crea el archivo binario y escribe las cuentas
void crear_cuentas(const char *archivo) {
    FILE *f = fopen(archivo, "wb");
    if (!f) {
        perror("No se pudo crear el archivo de cuentas");
        exit(EXIT_FAILURE);
    }

    Cuenta cuentas[] = {
        {1001, "John Doe", 5000.00, 0},
        {1002, "Jane Smith", 3000.00, 0},
        {1003, "Alice Johnson", 4500.50, 0},
        {1004, "Bob Brown", 6000.00, 0}
    };

    size_t num_cuentas = sizeof(cuentas) / sizeof(Cuenta);
    for (size_t i = 0; i < num_cuentas; i++) {
        fwrite(&cuentas[i], sizeof(Cuenta), 1, f);
    }

    fclose(f);
    printf("Archivo de cuentas creado exitosamente con %zu cuentas.\n", num_cuentas);
}

int main() {
    crear_cuentas("cuentas.dat");
    return 0;
}
