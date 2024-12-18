#include "prototipos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>

#define USUARIOS_LOG_PATH "/var/log/shell/usuario_horarios_log.log"
#define MOVIMIENTOS_LOG_PATH "/var/log/shell/shell_movimientos.log"
#define ERRORES_LOG_PATH "/var/log/shell/sistema_error.log"


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

    // Cambiar los permisos del archivo a 777
    chmod(MOVIMIENTOS_LOG_PATH, 0777);

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





// Función para obtener el timestamp actual
void obtener_timestamp(char *buffer, size_t buffer_size) {
    // Obtener el tiempo actual
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    // Formatear el timestamp como 'YYYY-MM-DD hh:mm:ss'
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Funcion para obtener la IP actual del usuario
void obtener_ip_actual(char *ip_buffer, size_t buffer_size) {
    // se obtiene la direccion de la red local, la segunda direccion ip...
    FILE *fp = popen("ip addr show | awk '/inet / {print $2}' | cut -d'/' -f1 | sed -n '2p'", "r");

    if (fp == NULL) {
        strncpy(ip_buffer, "desconocido", buffer_size);
        return;
    }

    fgets(ip_buffer, buffer_size, fp);
    ip_buffer[strcspn(ip_buffer, "\n")] = '\0'; // Eliminar salto de línea
    pclose(fp);
}

// Funcion para validar el inicio de sesión
// Funcion para comparar el usuario en el archivo
void validar_inicio_sesion(const char *usuario, const char *ip_actual, const char *horario_actual, FILE *log_file) {
    char linea[256];
    FILE *usuarios_file = fopen("/usr/local/bin/usuarios_agregados.txt", "r");
    if (usuarios_file == NULL) {
        fprintf(log_file, "Error: No se pudo abrir el archivo de usuarios para validar.\n");
        return;
    }

    char usuario_guardado[50], horario_guardado[50], ips_guardadas[200];
    int encontrado = 0;

    while (fgets(linea, sizeof(linea), usuarios_file)) {
        // Limpiar la cadena de los saltos de línea y espacios al final
        linea[strcspn(linea, "\n")] = 0; // Eliminar salto de línea al final

        if (sscanf(linea, "Usuario: %s\nHorario: %s\nIPs permitidas: %[^\n]\n",
                   usuario_guardado, horario_guardado, ips_guardadas) == 3) {
            // Limpiar espacios adicionales al final de las cadenas
            usuario_guardado[strcspn(usuario_guardado, " ")] = 0; // Eliminar espacios
            horario_guardado[strcspn(horario_guardado, " ")] = 0; // Eliminar espacios

            if (strcmp(usuario, usuario_guardado) == 0) {
                encontrado = 1;

                // Validar horario
                if (strcmp(horario_actual, horario_guardado) != 0) {
                    fprintf(log_file, "Advertencia: Usuario '%s' inicio sesion fuera del horario permitido. Horario actual: %s, Horario permitido: %s\n",
                            usuario, horario_actual, horario_guardado);
                }

                // Validar IP
                if (strstr(ips_guardadas, ip_actual) == NULL) {
                    fprintf(log_file, "Advertencia: Usuario '%s' inicio sesion desde una IP no permitida: %s. IPs permitidas: %s\n",
                            usuario, ip_actual, ips_guardadas);
                }
                break;
            }
        }
    }

    if (!encontrado) {
        //fprintf(log_file, "Advertencia: Usuario '%s' no está registrado en el sistema.\n", usuario);
        printf("\n");
    }

    fclose(usuarios_file);
}



// Función para registrar inicio y cierre de sesión con validación
void registrar_sesion(const char *usuario, const char *accion, const char *ip_actual, const char *horario_actual) {
    // Crear el directorio /var/log/shell si no existe
    struct stat st;
    if (stat("/var/log/shell", &st) == -1) {
        if (mkdir("/var/log/shell", 0777) != 0) {
            printf("Error al crear el directorio /var/log/shell: %s\n", strerror(errno));
            return;
        }
    }

    // Verificar si el archivo de log existe, si no, crearlo con permisos 777
    FILE *log_file = fopen(USUARIOS_LOG_PATH, "a");
    if (log_file == NULL) {
        printf("Error al abrir el archivo de log '%s': %s\n", USUARIOS_LOG_PATH, strerror(errno));
        return;
    }

    chmod(USUARIOS_LOG_PATH, 0777); // Asegurar que el archivo tenga permisos 777

    // Obtener el timestamp actual
    char timestamp[64];
    obtener_timestamp(timestamp, sizeof(timestamp));

    // Registrar la acción
    fprintf(log_file, "%s: Usuario '%s' %s sesion desde IP '%s' en horario '%s'.\n",
            timestamp, usuario, accion, ip_actual, horario_actual);

    // Validar horario e IP
    validar_inicio_sesion(usuario, ip_actual, horario_actual, log_file);

    fclose(log_file);
}
