#include <stdio.h>      // Para entrada/salida estándar
#include <stdlib.h>     // Para funciones generales (malloc, free, etc...)
#include <string.h>     // Para manipulación de cadenas (strtok, strcmp, etc...)
#include <unistd.h>     // Para funciones del sistema 
#include <sys/stat.h>   // Para manipulación de directorios
#include <errno.h>      // Para obtener mensajes de error del sistema
#include <dirent.h>     // Para opendir(), readdir(), closedir()
#include <pwd.h>       // Para verificar si el usuario existe
#include <grp.h>       // Para obtener información del grupo
#include <time.h>      // Lo usamos para ontener el tiempo actual en el sistema
#include <fcntl.h>     // Para operaciones con archivos
#include <libgen.h>    // Necesario para el comando 'copiar'
#include "prototipos.h"

#define BUFFER_SIZE 4096 //tamaño del buffer que se utiliza para leer y escribir bloques de datos durante la copia del archivo.


/* 
 * Función: crear_directorio
 * -------------------------
 * Crea un nuevo directorio con permisos 0777 (lectura, escritura, ejecución para todos)
 */
void crear_directorio(const char *nombre_directorio) {
    // Intentar crear el directorio
    if (mkdir(nombre_directorio, 0755) == 0) {
        printf("Directorio '%s' creado con éxito.\n", nombre_directorio);
    } else {
        // Mostrar un mensaje si ocurre un error al crear el directorio
        printf("Error al crear el directorio '%s': %s\n", nombre_directorio, strerror(errno));
    }
}

/*
 * Función: listar_directorios
 * ---------------------------
 * Lee y muestra en pantalla todos los directorios registrados en el archivo 'directorios.txt'.
 *
 * Comportamiento:
 *   Si el archivo no existe o no contiene información, muestra un mensaje al usuario.
 */

// Función para listar los directorios creados o el contenido de un directorio
void listar_directorios(const char *ruta) {
    DIR *dir;
    struct dirent *entry;

    // Si no se especifica una ruta, usamos el directorio actual
    if (ruta == NULL) {
        ruta = ".";
    }

    dir = opendir(ruta);
    if (dir == NULL) {
        printf("Error al abrir el directorio '%s', verifique su existencia\n", ruta);

        // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al abrir el directorio '%s', verifique su existencia\n", ruta);
        registrar_error(mensaje);  // Registrar en el log

        return;
    }

    //printf("Contenido del directorio '%s':\n", ruta);
    while ((entry = readdir(dir)) != NULL) {
        // Ignoramos las entradas especiales "." y ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("- %s\n", entry->d_name);
        }
    }

    closedir(dir);
}


/*
 * funcion "ir"
 * ---------------------------
 * Cambia el directorio actual a uno especificado por el usuario.
 *   Si el cambio es exitoso, muestra el nuevo directorio actual.
 *   Si ocurre un error, muestra un mensaje descriptivo.
 */
void cambiar_directorio(const char *nombre_directorio) {
    // Intentar cambiar al directorio especificado
    if (chdir(nombre_directorio) == 0) {
        printf("Cambiado al directorio '%s'.\n", nombre_directorio);

        // Obtener y mostrar el nuevo directorio actual
        char ruta_actual[256];
        if (getcwd(ruta_actual, sizeof(ruta_actual)) != NULL) {
            printf("Directorio actual: %s\n", ruta_actual);
        } else {
            printf("Error al obtener el directorio actual: %s\n", strerror(errno));
        }
    } else {
        // Mostrar un mensaje si ocurre un error al cambiar de directorio
        printf("Error al cambiar al directorio '%s', verifique su existencia\n", nombre_directorio);

        // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al cambiar al directorio '%s', verifique su existencia\n", nombre_directorio);
        registrar_error(mensaje);  // Registrar en el log
    }
}



// Función para renombrar un archivo o directorio
void renombrar_archivo(const char *nombre_actual, const char *nuevo_nombre) {
    // Renombrar el directorio o archivo en el sistema de archivos
    if (rename(nombre_actual, nuevo_nombre) == 0) {
        printf("El archivo o directorio '%s' ha sido renombrado a '%s'.\n", nombre_actual, nuevo_nombre);
    } else {
        printf("Error al renombrar '%s' a '%s', verifique su existencia\n", nombre_actual, nuevo_nombre);

        // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al renombrar '%s' a '%s', verifique su existencia\n", nombre_actual, nuevo_nombre);
        registrar_error(mensaje);  // Registrar en el log
    }
}



// Función para mover un archivo o directorio
void mover_archivo_o_directorio(const char *origen, const char *destino) {
    struct stat info_destino;
    char ruta_final[512];

    // Comprobar si el destino es un directorio
    if (stat(destino, &info_destino) == 0 && S_ISDIR(info_destino.st_mode)) {
        // Construir la ruta completa (directorio destino + nombre del archivo o directorio origen)
        snprintf(ruta_final, sizeof(ruta_final), "%s/%s", destino, strrchr(origen, '/') ? strrchr(origen, '/') + 1 : origen);
    } else {
        // Si el destino no es un directorio, usarlo tal cual
        strncpy(ruta_final, destino, sizeof(ruta_final));
    }

    // Intentar mover el archivo o directorio
    if (rename(origen, ruta_final) == 0) {
        printf("Archivo o directorio '%s' movido a '%s'.\n", origen, ruta_final);
    } else {
        printf("Error al mover '%s' a '%s', verifique si el contenido a mover o el destino exista\n", origen, ruta_final);

        // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al mover '%s' a '%s', verifique si el contenido a mover o el destino exista\n", origen, ruta_final);
        registrar_error(mensaje);  // Registrar en el log
    }
}




/*
 * Función: copiar
 * -----------------------
 * Copia un archivo o directorio de origen a destino.
 * Si el destino es un directorio, lo copia dentro de él.
 */
// Función para copiar archivos
void copiar_archivo(const char *origen, const char *destino) {
    int src_fd, dest_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    struct stat dest_stat;
    char final_destino[1024];

    // Verificar si el destino es un directorio
    if (stat(destino, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
        // Si es un directorio, concatenar el nombre del archivo origen al destino
        snprintf(final_destino, sizeof(final_destino), "%s/%s", destino, basename((char *)origen));
    } else {
        // Si no es un directorio, usar el destino tal cual
        strncpy(final_destino, destino, sizeof(final_destino) - 1);
        final_destino[sizeof(final_destino) - 1] = '\0';
    }

    // Abrir el archivo de origen en modo solo lectura
    src_fd = open(origen, O_RDONLY);
    if (src_fd < 0) {
        return;
    }

    // Abrir o crear el archivo de destino en modo escritura
    dest_fd = open(final_destino, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        close(src_fd);
        return;
    }

    // Leer del archivo origen y escribir en el archivo de destino en bloques de tamaño BUFFER_SIZE
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            close(src_fd);
            close(dest_fd);
            return;
        }
    }

    // Cerrar los archivos de origen y destino
    close(src_fd);
    close(dest_fd);

    // Imprimir un mensaje indicando que el archivo ha sido copiado
    printf("Archivo copiado de '%s' a '%s'.\n", origen, final_destino);
}

// Función para copiar un directorio de forma recursiva
void copiar_directorio(const char *origen, const char *destino) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char src_path[PATH_MAX];
    char dest_path[PATH_MAX];


    // // Crea el directorio de destino si no existe, tira error si no se pudo
    if (mkdir(destino, 0755) < 0 && errno != EEXIST) {
        //registrar_error("Error al crear el directorio de destino");
        return;
    }

    // Abre el directorio de origen
    dir = opendir(origen);
    if (dir == NULL) {
        //registrar_error("Error al abrir el directorio de origen");
        return;
    }

    // se lee las entradas del directorio de origen
    while ((entry = readdir(dir)) != NULL) {
        // Ignora las entradas "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construye las rutas completas de origen y destino
        snprintf(src_path, sizeof(src_path), "%s/%s", origen, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", destino, entry->d_name);

        // Obtiene información sobre la entrada actual
        if (stat(src_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                // Si es un directorio, copiar recursivamente
                copiar_directorio(src_path, dest_path);
            } else if (S_ISREG(statbuf.st_mode)) {
                // Si es un archivo, copiar
                copiar_archivo(src_path, dest_path);
            }
        }
    }

    // Cierra el directorio de origen
    closedir(dir);

    // Imprime un mensaje indicando que el directorio ha sido copiado
    printf("Directorio copiado de '%s' a '%s'.\n", origen, destino);
}


// Función para decidir si se copia un archvo o un directorio
void copiar(const char *origen, const char *destino) {
    struct stat statbuf;

    // Obtiene información sobre el origen
    if (stat(origen, &statbuf) < 0) {
        
        printf("Error al obtener información del origen, verifica si existe archivo o directorio.\n");

         // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al obtener información del origen, verifica si existe archivo o directorio.\n");
        registrar_error(mensaje);  // Registrar en el log
        return;
    }


    // Verifica si el origen es un directorio
    if (S_ISDIR(statbuf.st_mode)) {
        // Es un directorio: copiar recursivamente
        copiar_directorio(origen, destino);
    } else if (S_ISREG(statbuf.st_mode)) {
        // Es un archivo: copiar
        copiar_archivo(origen, destino);
    } else {
        // El origen no es un archivo ni un directorio válido
        printf("El origen '%s' no es un archivo ni un directorio valido.\n", origen);
    }
}






// Cambiar permisos de un archivo o recursivamente en un directorio
void cambiar_permisos_recursivo(const char *ruta, mode_t permisos) {
    struct stat st;
    
    // Obtener información sobre la ruta
    if (stat(ruta, &st) == -1) {
        printf("Error al acceder a '%s': %s\n", ruta, strerror(errno));
        return;
    }

    // Si es un archivo, cambiar los permisos
    if (S_ISREG(st.st_mode)) {
        if (chmod(ruta, permisos) == 0) {
            printf("Permisos de '%s' cambiados a '%o'.\n", ruta, permisos);
        } else {
            printf("Error al cambiar permisos de '%s': %s\n", ruta, strerror(errno));
        }
    } else {
        printf("Advertencia: '%s' no es un archivo regular. Se omitirá.\n", ruta);

         // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Advertencia: '%s' no es un archivo regular. Se omitirá.\n", ruta);
        registrar_error(mensaje);  // Registrar en el log
    }
    
}

// Función para manejar múltiples rutas
void cambiar_permisos(const char *modo, char **archivos, int num_archivos) {
    char *endptr;
    mode_t permisos = strtol(modo, &endptr, 8);

    if (*endptr != '\0' || permisos > 0777 || permisos < 0) {
        printf("Error: El modo '%s' no es valido..\n", modo);

        // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error: El modo '%s' no es valido.\n", modo);
        registrar_error(mensaje);  // Registrar en el log

        return;
    }

    for (int i = 0; i < num_archivos; i++) {
        cambiar_permisos_recursivo(archivos[i], permisos);
    }
}




// Función para obtener el UID del usuario
uid_t obtener_uid(const char *nombre_usuario) {
    struct passwd *pwd = getpwnam(nombre_usuario);
    if (pwd == NULL) {
        return -1; // Usuario no encontrado
    }
    return pwd->pw_uid;
}

// Función para obtener el GID del grupo
gid_t obtener_gid(const char *nombre_grupo) {
    struct group *grp = getgrnam(nombre_grupo);
    if (grp == NULL) {
        return -1; // Grupo no encontrado
    }
    return grp->gr_gid;
}

void cambiar_propietario_y_grupo(const char *nombre_usuario, const char *nombre_grupo, char **archivos, int num_archivos) {
    uid_t uid = -1; // Inicializamos con -1 (sin cambio)
    gid_t gid = -1; // Inicializamos con -1 (sin cambio)

    // Si el nombre de usuario no es "-" y no es NULL, obtener el UID
    if (nombre_usuario != NULL && strcmp(nombre_usuario, "-") != 0) {
        uid = obtener_uid(nombre_usuario);
        if (uid == (uid_t)-1) {
            printf("Error: El usuario '%s' no existe.\n", nombre_usuario);

            // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
            char mensaje[256];
            snprintf(mensaje, sizeof(mensaje), "Error: El usuario '%s' no existe.\n", nombre_usuario);
            registrar_error(mensaje);  // Registrar en el log

            return;
        }
    }

    // Si el nombre de grupo no es "-" y no es NULL, obtener el GID
    if (nombre_grupo != NULL && strcmp(nombre_grupo, "-") != 0) {
        gid = obtener_gid(nombre_grupo);
        if (gid == (gid_t)-1) {
            printf("Error: El grupo '%s' no existe.\n", nombre_grupo);

            // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
            char mensaje[256];
            snprintf(mensaje, sizeof(mensaje), "Error: El grupo '%s' no existe.\n", nombre_grupo);
            registrar_error(mensaje);  // Registrar en el log

            return;
        }
    }

    // Cambiar propietario y grupo para cada archivo/directorio
    for (int i = 0; i < num_archivos; i++) {
        if (chown(archivos[i], uid, gid) == 0) {
            printf("Propietario de '%s' cambiado a '%s' y grupo a '%s'.\n",
                   archivos[i],
                   (nombre_usuario && strcmp(nombre_usuario, "-") != 0) ? nombre_usuario : "(sin cambio)",
                   (nombre_grupo && strcmp(nombre_grupo, "-") != 0) ? nombre_grupo : "(sin cambio)");
        } else {
            printf("Error al cambiar propietario o grupo de '%s': %s\n", archivos[i], strerror(errno));

            // esto es para ir agregando los errores que se le presentan al usuario e ir guardando en sistema_error.log
            char mensaje[256];
            snprintf(mensaje, sizeof(mensaje), "Error al cambiar propietario o grupo de '%s': %s\n", archivos[i], strerror(errno));
            registrar_error(mensaje);  // Registrar en el log
        }
    }
}






// Función para verificar si un usuario ya existe
int usuario_existe(const char *nombre_usuario) {
    struct passwd *pwd = getpwnam(nombre_usuario);
    return (pwd != NULL); // Retorna 1 si el usuario existe, 0 si no
}


// Función para agregar un nuevo usuario con contraseña, horario y lugares de conexión
void agregar_usuario(const char *nombre_usuario, const char *contrasena, const char *horario, const char *lugares_conexion) {
    if (usuario_existe(nombre_usuario)) {
        printf("Error: El usuario '%s' ya existe.\n", nombre_usuario);
        return;
    }

    // Separar el horario en hora de entrada y hora de salida
    char hora_entrada[6], hora_salida[6];
    sscanf(horario, "%5[^,],%5s", hora_entrada, hora_salida);

    // Crear el usuario en el sistema
    char comando[256];
    snprintf(comando, sizeof(comando), "useradd -m %s", nombre_usuario);
    if (system(comando) != 0) {
        printf("Error al agregar el usuario '%s'. Verifica si tienes permisos de root.\n", nombre_usuario);
        return;
    }
    
    // Establecer la contraseña
    snprintf(comando, sizeof(comando), "echo '%s:%s' | chpasswd", nombre_usuario, contrasena);
    if (system(comando) != 0) {
        printf("Error al establecer la contrasena para '%s'.\n", nombre_usuario);
        return;
    }

    // Guardar los datos del usuario en el archivo
    FILE *archivo = fopen("/usr/local/bin/usuarios_agregados.txt", "a");
    if (archivo == NULL) {
        printf("Error al abrir el archivo para registrar datos adicionales.\n");
        return;
    }

    // Guardar los datos en el formato: nombre_usuario|hora_entrada,hora_salida|IPs_permitidas
    fprintf(archivo, "%s|%s|%s\n", nombre_usuario, horario, lugares_conexion);
    fclose(archivo);

    printf("Usuario '%s' agregado con exito, contrasena establecida y datos registrados.\n", nombre_usuario);
}








// Función para cambiar la contraseña de un usuario, root puede cambiar la contrasena de aualquier usuario, pero un usuario normal solo podra cambiar el suyo.
void cambiar_contrasena(const char *nombre_usuario, const char *nueva_contrasena) {
    // Obtener el usuario actual
    const char *usuario_actual = getenv("USER");
    if (usuario_actual == NULL) {
        printf("Error: No se pudo determinar el usuario actual.\n");
        return;
    }

    // Verificar si el usuario actual es root o el mismo usuario
    if (strcmp(usuario_actual, "root") == 0 || strcmp(usuario_actual, nombre_usuario) == 0) {
        if (strcmp(usuario_actual, nombre_usuario) == 0 && strcmp(usuario_actual, "root") != 0) {
            // El usuario no root cambiará su propia contraseña interactivamente
            printf("Cambiando la contrasena del usuario '%s'. Por favor, introduce la nueva contrasena.\n", usuario_actual);

            char comando[256];
            snprintf(comando, sizeof(comando), "passwd");

            // Ejecutar el comando passwd sin el nombre del usuario
            int resultado = system(comando);
            if (resultado == 0) {
                printf("Contrasena para el usuario '%s' cambiada con exito.\n", usuario_actual);

                // Registrar en el log
                char mensaje[256];
                snprintf(mensaje, sizeof(mensaje), "Contrasena para el usuario '%s' cambiada con exito.\n", usuario_actual);
                registrar_error(mensaje);
            } else {
                printf("Error al cambiar la contrasena para el usuario '%s'.\n", usuario_actual);

                // Registrar en el log
                char mensaje[256];
                snprintf(mensaje, sizeof(mensaje), "Error al cambiar la contrasena para el usuario '%s'.\n", usuario_actual);
                registrar_error(mensaje);
            }
        } else {
            // El usuario root cambia contraseñas sin restricciones
            if (nueva_contrasena == NULL || strlen(nueva_contrasena) == 0) {
                printf("Error: Debes proporcionar la nueva contrasena para el usuario '%s'.\n", nombre_usuario);
                return;
            }

            char comando[256];
            snprintf(comando, sizeof(comando), "echo '%s:%s' | chpasswd", nombre_usuario, nueva_contrasena);

            // Ejecutar el comando
            int resultado = system(comando);
            if (resultado == 0) {
                printf("Contrasena para el usuario '%s' cambiada con exito.\n", nombre_usuario);

                // Registrar en el log
                char mensaje[256];
                snprintf(mensaje, sizeof(mensaje), "Contrasena para el usuario '%s' cambiada con exito.\n", nombre_usuario);
                registrar_error(mensaje);
            } else {
                printf("Error al cambiar la contrasena para el usuario '%s'.\n", nombre_usuario);

                // Registrar en el log
                char mensaje[256];
                snprintf(mensaje, sizeof(mensaje), "Error al cambiar la contrasena para el usuario '%s'.\n", nombre_usuario);
                registrar_error(mensaje);
            }
        }
    } else {
        // Si el usuario no es root y no coincide con la cuenta actual
        printf("Error: No tienes permisos para cambiar la contrasena de otros usuarios.\n");

        // Registrar en el log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error: Usuario '%s' intento cambiar la contrasena de '%s' sin permisos.\n", usuario_actual, nombre_usuario);
        registrar_error(mensaje);
    }
}









/*
 * Función principal
 * Proporciona una terminal interactiva que permite al usuario ejecutar los comandos:
 * En esta main estan todo los comandos solicitados en el tp
 * Comportamiento:
 *   Muestra un prompt (`>`) para que el usuario ingrese comandos.
 *   El programa continúa ejecutándose hasta que el usuario escribe 'salir'.
 */

int main() {
    char comando[256];
    char *accion, *argumentos[10];  // Permitir hasta 10 argumentos (puedes ajustar este límite)
    int num_argumentos;
    // se usa para hacer el registro de inicio y cierre de sesion
    char ip_actual[50];
    char horario_actual[50];
    char *usuario = getenv("USER");  // con esto obtenemos el 'nombre' del usuario.

    obtener_ip_actual(ip_actual, sizeof(ip_actual)); // Para obtener su Ip actual
    obtener_timestamp(horario_actual, sizeof(horario_actual)); // Usar esta función para obtener la hora actual

    // registra el inicio de sesion
    registrar_sesion(usuario, "inicio", ip_actual, horario_actual, 0);


    //Bienvenido a la terminal personalizada. Escribe 'salir' para terminar.\n
    printf("Bienvenido a la shell, si quiere cerrarlo escriba el comando 'salir'.\n");

    while (1) {
        // Mostrar el prompt
        printf("> ");
        fgets(comando, sizeof(comando), stdin);

        // Limpieza del comando ingresado
        comando[strcspn(comando, "\n")] = 0;  // Eliminar salto de línea
        while (strlen(comando) > 0 && comando[strlen(comando) - 1] == ' ') {
            comando[strlen(comando) - 1] = '\0';  // Eliminar espacios al final
        }

        // Registrar el comando ingresado por el usuario
        registrar_movimientos(comando);

        // Dividimos el comando en acción y argumentos
        accion = strtok(comando, " ");
        num_argumentos = 0;

        // Extraer todos los argumentos restantes
        char *arg = strtok(NULL, " ");
        while (arg != NULL && num_argumentos < 10) {  // Limitar a 10 argumentos
            argumentos[num_argumentos++] = arg;
            arg = strtok(NULL, " ");
        }

        if (accion) {
            if (strcmp(accion, "creardir") == 0) {
                if (num_argumentos >= 1) {
                    crear_directorio(argumentos[0]);
                } else {
                    printf("Error: No se proporciono un nombre para el directorio.\n");
                }
            } else if (strcmp(accion, "listar") == 0) {
                listar_directorios(num_argumentos >= 1 ? argumentos[0] : NULL);
            } else if (strcmp(accion, "ir") == 0) {
                if (num_argumentos >= 1) {
                    cambiar_directorio(argumentos[0]);
                } else {
                    printf("Error: No se proporciono un nombre para el directorio.\n");
                }
            } else if (strcmp(accion, "renombrar") == 0) {
                if (num_argumentos >= 2) {
                    renombrar_archivo(argumentos[0], argumentos[1]);
                } else {
                    printf("Error: Debes proporcionar el nombre actual y el nuevo nombre.\n");
                }
            } else if (strcmp(accion, "mover") == 0) {
                if (num_argumentos >= 2) {
                    mover_archivo_o_directorio(argumentos[0], argumentos[1]);
                } else {
                    printf("Error: Debes proporcionar el archivo o directorio origen y el destino.\n");
                }
            } else if (strcmp(accion, "copiar") == 0) {
                if (num_argumentos >= 2) {
                    copiar(argumentos[0], argumentos[1]);
                } else {
                    printf("Error: Debes proporcionar el archivo o directorio origen y el destino.\n");
                }
            }
            else if (strcmp(accion, "permisos") == 0) {
                if (num_argumentos >= 2) {
                    // Argumento 1: modo; Resto: archivos
                    cambiar_permisos(argumentos[0], &argumentos[1], num_argumentos - 1);
                } else {
                    printf("Error: Debes proporcionar el modo y al menos un archivo.\n");
                }
            } else if (strcmp(accion, "propietario") == 0) {
                if (num_argumentos >= 3) {
                    // Si hay un usuario y un grupo, cambiar ambos
                    cambiar_propietario_y_grupo(argumentos[0], argumentos[1], &argumentos[2], num_argumentos - 2);
                } else if (num_argumentos == 2) {
                    // Si solo hay un grupo, cambiar solo el grupo
                    cambiar_propietario_y_grupo(NULL, argumentos[1], &argumentos[2], num_argumentos - 2);
                } else {
                    printf("Error: Debes proporcionar un usuario y/o un archivo o directorio.\n");
                }
            } else if (strcmp(accion, "usuario") == 0) {
                if (num_argumentos >= 4) {  // Ahora esperamos 4 argumentos: nombre_usuario, contrasena, horario, lugares_conexion
                    agregar_usuario(argumentos[0], argumentos[1], argumentos[2], argumentos[3]);
                } else {
                    printf("Error: Debes proporcionar el nombre del usuario, contrasena, horario y lugares de conexion.\n");
                }
            }
            else if (strcmp(accion, "contrasena") == 0) {
                if (num_argumentos >= 2) {
                    cambiar_contrasena(argumentos[0], argumentos[1]); // Usuario y nueva contraseña
                } else {
                    printf("Error: Debes proporcionar el nombre del usuario y la nueva contraseña.\n");
                }
            } else if (strcmp(accion, "ejecutar") == 0) {
                if (num_argumentos >= 1) {
                    // Unir los argumentos en un solo comando
                    char comando_final[256] = "";
                    for (int i = 0; i < num_argumentos; i++) {
                        strcat(comando_final, argumentos[i]);
                        if (i < num_argumentos - 1) {
                            strcat(comando_final, " ");
                        }
                    }
                    // Ejecutar el comando
                    ejecutar_comando(comando_final);
                } else {
                    printf("Error: Debes proporcionar un comando del sistema para ejecutar.\n");
                }
            }
            else if (strcmp(accion, "demonio") == 0) {
                if (num_argumentos >= 2) {
                    gestionar_demonio(argumentos[0], argumentos[1]);
                } else {
                    printf("Error: Debes proporcionar una acción ('start' o 'stop') y un servicio.\n");
                }
            }

            else if (strcmp(accion, "transferir") == 0) {
                if (num_argumentos == 3) { // Protocolo, archivo_local, servidor:ruta_remota
                
                    if (strcmp(argumentos[0], "scp") == 0) {
                        ejecutar_transferencia_scp(argumentos[1], argumentos[2]);
                    } else {
                        printf("Error: Protocolo '%s' no soportado, solo se admite 'scp'.\n", argumentos[0]);
                    }
                } else {
                    printf("Error: Uso correcto: transferir scp <archivo_local> <usuario>@<servidor>:<ruta_remota>\n");
                }
            }


            else if (strcmp(accion, "salir") == 0) {
                break;
            } else {
                printf("Comando no reconocido: '%s'.\n", accion);
            }
        }
    }

    // Registrar cierre de sesión
    obtener_timestamp(horario_actual, sizeof(horario_actual));
    // el usuario cerro sesion
    registrar_sesion(usuario, "cerro", ip_actual, horario_actual, 1);

    return 0;
}
