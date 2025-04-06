#include <stdio.h>      // Para funciones de entrada/salida como fopen, fwrite, printf
#include <stdlib.h>     // Para funciones como exit()
#include <string.h>     // Para trabajar con strings

// Definimos la estructura de una cuenta bancaria
typedef struct {
    int numero_cuenta;         // Número único de cuenta
    char titular[50];          // Nombre del titular de la cuenta
    float saldo;               // Saldo actual de la cuenta
    int num_transacciones;     // Cantidad de transacciones realizadas
} Cuenta;

// Función que crea el archivo binario y escribe las cuentas
void crear_cuentas(const char *archivo) {
    // Abrimos el archivo en modo binario de escritura
    FILE *f = fopen(archivo, "wb");
    if (!f) {
        // Si falla la apertura, mostramos un error y terminamos el programa
        perror("No se pudo crear el archivo de cuentas");
        exit(EXIT_FAILURE);
    }

    // Creamos un array con varias cuentas de ejemplo
    Cuenta cuentas[] = {
        {1001, "John Doe", 5000.00, 0},
        {1002, "Jane Smith", 3000.00, 0},
        {1003, "Alice Johnson", 4500.50, 0},
        {1004, "Bob Brown", 6000.00, 0}
    };

    // Calculamos cuántas cuentas hay en el array
    size_t num_cuentas = sizeof(cuentas) / sizeof(Cuenta);

    // Escribimos cada cuenta en el archivo binario
    for (size_t i = 0; i < num_cuentas; i++) {
        fwrite(&cuentas[i], sizeof(Cuenta), 1, f);
    }

    // Cerramos el archivo una vez escritas todas las cuentas
    fclose(f);

    // Mensaje de éxito para el usuario
    printf("Archivo de cuentas creado exitosamente con %zu cuentas.\n", num_cuentas);
}

// Función principal del programa
int main() {
    const char *nombre_archivo = "cuentas.dat"; // Nombre del archivo a crear
    crear_cuentas(nombre_archivo);              // Llamamos a la función que lo crea
    return 0;                                   // Terminamos correctamente
}
