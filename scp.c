#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>   // Para manipulación de directorios
#include <unistd.h>     // Para funciones del sistema (chdir, getcwd)
#include <errno.h>      // Para obtener mensajes de error del sistema
#include "prototipos.h"

// Ruta del log para transferencias
#define LOG_TRANSFERENCIAS "/var/log/shell/shell_transferencias.log"

// Función para registrar transferencias en el log
void registrar_transferencia_log(const char *archivo_local, const char *destino, int exito) {
    struct stat st;

    // Crear el directorio /var/log/shell si no existe
    if (stat("/var/log/shell", &st) == -1) {
        if (mkdir("/var/log/shell", 0777) != 0) {
            printf("Error al crear el directorio /var/log/shell: %s\n", strerror(errno));
            return;
        }
    }

    FILE *log_file = fopen(LOG_TRANSFERENCIAS, "a");
    if (log_file == NULL) {
        printf("Error: No se pudo abrir el archivo de log '%s'.\n", LOG_TRANSFERENCIAS);
        return;
    }

    chmod(LOG_TRANSFERENCIAS, 0777); // Asegurar que el archivo tenga permisos 777

    // Obtener el timestamp actual
    char timestamp[64];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Registrar transferencia
    fprintf(log_file, "[%s] Transferencia %s: %s -> %s\n",
            timestamp,
            exito ? "EXITOSA" : "FALLIDA",
            archivo_local,
            destino);

    fclose(log_file);
}



// Función para ejecutar la transferencia con SCP
void ejecutar_transferencia_scp(const char *archivo_local, const char *destino) {
   
    // Comando SCP
    char comando[512];
    snprintf(comando, sizeof(comando), "scp %s %s", archivo_local, destino);

    // Ejecutar el comando SCP
    printf("Ejecutando: %s\n", comando);
    int resultado = system(comando);

    if (resultado == 0) {
        printf("Transferencia exitosa: %s -> %s\n", archivo_local, destino);
        registrar_transferencia_log(archivo_local, destino, 1);
    } else {
        printf("Error en la transferencia: %s -> %s\n", archivo_local, destino);
        registrar_transferencia_log(archivo_local, destino, 0);
    }
}
