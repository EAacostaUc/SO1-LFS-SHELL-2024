#include "prototipos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define LOG_PATH "/var/log/shell/sistema_error.log"

// Función para registrar un error en el archivo de log
void registrar_error(const char *mensaje) {
    struct stat st;

    // Crear el directorio /var/log/shell si no existe
    if (stat("/var/log/shell", &st) == -1) {
        mkdir("/var/log/shell", 0777);  // Permisos 777 para el directorio
    }

    // Abrir el archivo de log en modo de adición
    FILE *log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL) {
        printf("Error al abrir el archivo de log '%s': %s\n", LOG_PATH, strerror(errno));
        return;
    }

    // Cambiar los permisos del archivo a 777
    chmod(LOG_PATH, 0777);

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
