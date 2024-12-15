#include "prototipos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define ERRORES_LOG_PATH "/var/log/shell/sistema_error.log"
#define MOVIMIENTOS_LOG_PATH "/var/log/shell/shell_movimientos.log"

// Función para registrar un error en el archivo de log
void registrar_error(const char *mensaje) {
    struct stat st;

    // Crear el directorio /var/log/shell si no existe
    if (stat("/var/log/shell", &st) == -1) {
        mkdir("/var/log/shell", 0777);  // Permisos 777 para el directorio
    }

    // Abrir el archivo de log en modo de adición
    FILE *log_file = fopen(ERRORES_LOG_PATH, "a");
    if (log_file == NULL) {
        printf("Error al abrir el archivo de log '%s': %s\n", ERRORES_LOG_PATH, strerror(errno));
        return;
    }

    // Cambiar los permisos del archivo a 777
    chmod(ERRORES_LOG_PATH, 0777);

    // Obtener el timestamp actual
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timestamp[26];
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    // Escribir el mensaje en el log
    fprintf(log_file, "%s %s\n", timestamp, mensaje);

    // Cerrar el archivo
    fclose(log_file);
}





// Función para registrar movimientos realizados por el usuario
void registrar_movimientos(const char *comando) {
    FILE *log;
    char timestamp[20];  // Espacio para el timestamp
    char mensaje[512];   // Espacio para el mensaje formateado

    // Crear el directorio /var/log/shell si no existe
    struct stat st;
    if (stat("/var/log/shell", &st) == -1) {
        if (mkdir("/var/log/shell", 0777) != 0) {  // para que cualquier usuario pueda usarlo
            printf("Error al crear el directorio '/var/log/shell': %s\n", strerror(errno));
            return;
        }
    }

    // Obtener el timestamp actual
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Formatear el mensaje
    snprintf(mensaje, sizeof(mensaje), "%s - %s\n", timestamp, comando);

    // Abrir el archivo de log en modo de añadir (append)
    log = fopen(MOVIMIENTOS_LOG_PATH, "a");
    if (log != NULL) {
        // Escribir el timestamp y el comando en el archivo
        fprintf(log, "%s", mensaje);
        fclose(log);
    } else {
        printf("Error al abrir el archivo de log '%s': %s\n", MOVIMIENTOS_LOG_PATH, strerror(errno));
    }
}