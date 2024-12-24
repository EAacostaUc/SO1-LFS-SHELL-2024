#include "prototipos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>  // Para el uso de 'stat', para verificar existencia de archivos/directorios
#include <errno.h>    // Para obtener mensajes de error del sistema
#include <unistd.h>
#include <stddef.h>

#define USUARIOS_LOG_PATH "/var/log/shell/usuario_horarios_log.log"
#define MOVIMIENTOS_LOG_PATH "/var/log/shell/shell_movimientos.log"
#define ERRORES_LOG_PATH "/var/log/shell/sistema_error.log"


// Función para registrar un error en el archivo de log, ocasionados por los usuarios, por ej: al usar de forma incorrecta los comandos.
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

    // Formatear el timestamp como 'hh:mm'
    strftime(buffer, buffer_size, "%H:%M", tm_info);
}

// Funcion para obtener la IP actual del usuario
void obtener_ip_actual(char *ip_buffer, size_t buffer_size) {
    // se obtiene la direccion de la red local, la segunda direccion ip...
    FILE *fp = popen("hostname -i | awk '{print $1}'", "r");  // trabajamos con 'hostname -i' con esto no tenemos problemas en el LFS

    if (fp == NULL) {
        strncpy(ip_buffer, "desconocido", buffer_size);
        return;
    }

    fgets(ip_buffer, buffer_size, fp);
    ip_buffer[strcspn(ip_buffer, "\n")] = '\0'; // Eliminar salto de línea
    pclose(fp);
}



// Función para validar el inicio de sesión
void validar_inicio_sesion(const char *usuario, const char *ip_actual, const char *hora_entrada, const char *hora_salida, FILE *log_file, int es_salida) {
    char linea[256];
    FILE *usuarios_file = fopen("/usr/local/bin/usuarios_agregados.txt", "r"); // esto se cre recien cuando se agregan nuevos usuarios
    if (usuarios_file == NULL) {
        return;
    }

    char usuario_guardado[50], hora_entrada_guardada[50], hora_salida_guardada[50], ips_guardadas[256];
    int encontrado = 0;

    /* en este while lo que se hace es recorrer cada linea del contenido de usuarios_agregados.txt, y va guardando informacion de cada linea y revisando si existe el usurio
    que inicio sesion, si existe, antes de comparar si coinciden, ya se guarda la informacion del usuario, luego se compara y si coincide se procede a comparar el resto de informacion*/
    while (fgets(linea, sizeof(linea), usuarios_file)) {
        // Limpiar la cadena de los saltos de línea y espacios al final
        linea[strcspn(linea, "\n")] = 0; // Eliminar salto de línea al final

        if (sscanf(linea, "%[^|]|%[^,],%[^|]|%[^\n]", usuario_guardado, hora_entrada_guardada, hora_salida_guardada, ips_guardadas) == 4) {
            // Limpiar espacios adicionales al final de las cadenas
            usuario_guardado[strcspn(usuario_guardado, " ")] = 0; // Eliminar espacios
            hora_entrada_guardada[strcspn(hora_entrada_guardada, " ")] = 0; // Eliminar espacios
            hora_salida_guardada[strcspn(hora_salida_guardada, " ")] = 0; // Eliminar espacios

            // esto si coinciden, significa que el usuario que inicio sesion fue agregado antes...
            if (strcmp(usuario, usuario_guardado) == 0) {
                encontrado = 1;

                // Validar la IP
                if (strstr(ips_guardadas, ip_actual) == NULL) {
<<<<<<< HEAD
                    fprintf(log_file, "Usuario '%s' inicio sesion desde una IP no permitida: %s. IP permitidas: %s\n",
=======
                    fprintf(log_file, "Usuario '%s' inicio sesion desde una IP no permitida: %s. IP permitida: %s\n",
>>>>>>> a6689c6bd541a97c22c243d6cf355fc3e198b8dd
                            usuario, ip_actual, ips_guardadas);
                }

                // Validar hora de entrada al ingresar
                if (!es_salida && strcmp(hora_entrada, hora_entrada_guardada) != 0) {
                    fprintf(log_file, "Usuario '%s' inicio sesion fuera del horario permitido. Hora actual: %s, Hora de entrada permitida: %s\n",
                            usuario, hora_entrada, hora_entrada_guardada);
                }

                // Validar hora de salida al salir
                if (es_salida && strcmp(hora_salida, hora_salida_guardada) != 0) {
                    fprintf(log_file, "Usuario '%s' cerro sesion fuera del horario permitido. Hora actual: %s, Hora de salida permitida: %s\n",
                            usuario, hora_salida, hora_salida_guardada);
                }

                break;
            }
        }
    }
    // cuando el usuario que ingreso no ha sido agregado previamente, no esta en "usuarios_agregados.txt", en el caso del 'root' por ejemplo.
    if (!encontrado) {
        fprintf(log_file, "Usuario '%s' no esta registrado en el sistema.\n", usuario);
    }

    fclose(usuarios_file);
}

// Función para registrar inicio y cierre de sesión con validación
void registrar_sesion(const char *usuario, const char *accion, const char *ip_actual, const char *hora_actual, int es_salida) {
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
    fprintf(log_file, "Usuario '%s' %s sesion desde IP '%s' en hora '%s'.\n", usuario, accion, ip_actual, hora_actual);

    // Validar horario e IP
    validar_inicio_sesion(usuario, ip_actual, hora_actual, hora_actual, log_file, es_salida);

    fclose(log_file);
}
