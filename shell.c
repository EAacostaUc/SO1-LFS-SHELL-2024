#include <stdio.h>      // Para entrada/salida estándar
#include <stdlib.h>     // Para funciones generales (malloc, free, etc.)
#include <string.h>     // Para manipulación de cadenas (strtok, strcmp, etc.)
#include <unistd.h>     // Para funciones del sistema (chdir, getcwd)
#include <sys/stat.h>   // Para manipulación de directorios (mkdir)
#include <errno.h>      // Para obtener mensajes de error del sistema
#include <dirent.h>     // Para opendir(), readdir(), closedir()
#include <pwd.h>       // Para verificar si el usuario existe
#include <grp.h>       // Para obtener información del grupo
#include "prototipos.h"


/* holaaaaa
 * Función: crear_directorio
 * -------------------------
 * Crea un nuevo directorio con permisos 0777 (lectura, escritura, ejecución para todos).
 * También guarda el nombre del directorio en un archivo llamado 'directorios.txt'.
 *
 * Parámetros:
 *   nombre_directorio - Nombre del directorio a crear.
 *
 * Comportamiento:
 *   Si el directorio se crea con éxito, se registra en el archivo 'directorios.txt'.
 *   Si ocurre un error, se muestra un mensaje descriptivo.
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
        printf("Error al abrir el directorio '%s': %s\n", ruta, strerror(errno));
        return;
    }

    printf("Contenido del directorio '%s':\n", ruta);
    while ((entry = readdir(dir)) != NULL) {
        // Ignoramos las entradas especiales "." y ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("- %s\n", entry->d_name);
        }
    }

    closedir(dir);
}


/*
 * Función: cambiar_directorio
 * ---------------------------
 * Cambia el directorio actual a uno especificado por el usuario.
 *
 * Parámetros:
 *   nombre_directorio - Nombre del directorio al que se desea cambiar.
 *
 * Comportamiento:
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
        printf("Error al cambiar al directorio '%s': %s\n", nombre_directorio, strerror(errno));
    }
}



// Función para renombrar un archivo o directorio
void renombrar_archivo(const char *nombre_actual, const char *nuevo_nombre) {
    // Renombrar el directorio o archivo en el sistema de archivos
    if (rename(nombre_actual, nuevo_nombre) == 0) {
        printf("El archivo o directorio '%s' ha sido renombrado a '%s'.\n", nombre_actual, nuevo_nombre);
    } else {
        printf("Error al renombrar '%s' a '%s': %s\n", nombre_actual, nuevo_nombre, strerror(errno));
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
        printf("Error al mover '%s' a '%s': %s\n", origen, ruta_final, strerror(errno));
    }
}


// Función para copiar un archivo de un lugar a otro
void copiar_archivo(const char *origen, const char *destino) {
    FILE *f_origen, *f_destino;
    char buffer[1024];
    size_t bytes;
    char ruta_final[512];
    struct stat info_destino;

    // Comprobar si el destino es un directorio
    if (stat(destino, &info_destino) == 0 && S_ISDIR(info_destino.st_mode)) {
        // Construir la ruta completa (directorio destino + nombre del archivo origen)
        snprintf(ruta_final, sizeof(ruta_final), "%s/%s", destino, strrchr(origen, '/') ? strrchr(origen, '/') + 1 : origen);
    } else {
        // Si el destino no es un directorio, usarlo tal cual
        strncpy(ruta_final, destino, sizeof(ruta_final));
    }

    // Abrir el archivo origen en modo lectura binaria
    f_origen = fopen(origen, "rb");
    if (f_origen == NULL) {
        printf("Error al abrir el archivo origen '%s': %s\n", origen, strerror(errno));
        return;
    }

    // Abrir el archivo destino en modo escritura binaria
    f_destino = fopen(ruta_final, "wb");
    if (f_destino == NULL) {
        printf("Error al crear el archivo destino '%s': %s\n", ruta_final, strerror(errno));
        fclose(f_origen);
        return;
    }

    // Copiar el contenido del archivo origen al archivo destino
    while ((bytes = fread(buffer, 1, sizeof(buffer), f_origen)) > 0) {
        fwrite(buffer, 1, bytes, f_destino);
    }

    printf("Archivo '%s' copiado a '%s'.\n", origen, ruta_final);

    // Cerrar los archivos
    fclose(f_origen);
    fclose(f_destino);
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
    }
    // Si es un directorio, aplicar recursivamente
    else if (S_ISDIR(st.st_mode)) {
        if (chmod(ruta, permisos) == 0) {
            printf("Permisos del directorio '%s' cambiados a '%o'.\n", ruta, permisos);
        } else {
            printf("Error al cambiar permisos del directorio '%s': %s\n", ruta, strerror(errno));
        }

        // Abrir el directorio y aplicar a su contenido
        DIR *dir = opendir(ruta);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                // Ignorar las entradas `.` y `..`
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    // Construir la ruta completa del archivo o subdirectorio
                    char ruta_completa[512];
                    snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta, entry->d_name);
                    cambiar_permisos_recursivo(ruta_completa, permisos);
                }
            }
            closedir(dir);
        } else {
            printf("Error al abrir el directorio '%s': %s\n", ruta, strerror(errno));
        }
    }
}

// Función para manejar múltiples rutas
void cambiar_permisos(const char *modo, char **archivos, int num_archivos) {
    char *endptr;
    mode_t permisos = strtol(modo, &endptr, 8);

    if (*endptr != '\0' || permisos > 0777 || permisos < 0) {
        printf("Error: El modo '%s' no es válido. Debe ser un número octal (ejemplo: 644, 755).\n", modo);
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
            return;
        }
    }

    // Si el nombre de grupo no es "-" y no es NULL, obtener el GID
    if (nombre_grupo != NULL && strcmp(nombre_grupo, "-") != 0) {
        gid = obtener_gid(nombre_grupo);
        if (gid == (gid_t)-1) {
            printf("Error: El grupo '%s' no existe.\n", nombre_grupo);
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
        }
    }
}






// Función para verificar si un usuario ya existe
int usuario_existe(const char *nombre_usuario) {
    struct passwd *pwd = getpwnam(nombre_usuario);
    return (pwd != NULL); // Retorna 1 si el usuario existe, 0 si no
}


// Función para agregar un nuevo usuario con contraseña
void agregar_usuario(const char *nombre_usuario, const char *contrasena, const char *datos_personales, const char *horario, const char *lugares_conexion) {
    if (usuario_existe(nombre_usuario)) {
        printf("Error: El usuario '%s' ya existe.\n", nombre_usuario);
        return;
    }

    // Crear el usuario en el sistema
    char comando[256];
    snprintf(comando, sizeof(comando), "useradd %s", nombre_usuario);
    if (system(comando) != 0) {
        printf("Error al crear el usuario '%s'. Verifica si tienes permisos de root.\n", nombre_usuario);
        return;
    }

    // Establecer la contraseña del usuario
    snprintf(comando, sizeof(comando), "echo '%s:%s' | chpasswd", nombre_usuario, contrasena);
    if (system(comando) != 0) {
        printf("Error al establecer la contraseña para el usuario '%s'.\n", nombre_usuario);
        return;
    }

    // Registrar los datos adicionales
    FILE *archivo = fopen("usuarios_extra.txt", "a");
    if (archivo == NULL) {
        printf("Error al abrir el archivo para registrar datos adicionales.\n");
        return;
    }

    fprintf(archivo, "Usuario: %s\nContraseña: %s\nDatos Personales: %s\nHorario: %s\nLugares de conexión: %s\n\n",
            nombre_usuario, contrasena, datos_personales, horario, lugares_conexion);
    fclose(archivo);

    printf("Usuario '%s' creado con éxito, contraseña establecida y datos registrados.\n", nombre_usuario);
}



// Función para cambiar la contraseña de un usuario
void cambiar_contrasena(const char *nombre_usuario, const char *nueva_contrasena) {
    if (!usuario_existe(nombre_usuario)) {
        printf("Error: El usuario '%s' no existe.\n", nombre_usuario);
        return;
    }

    // Crear el comando para cambiar la contraseña usando `chpasswd`
    char comando[256];
    snprintf(comando, sizeof(comando), "echo '%s:%s' | chpasswd", nombre_usuario, nueva_contrasena);

    // Ejecutar el comando
    int resultado = system(comando);
    if (resultado == 0) {
        printf("Contraseña para el usuario '%s' cambiada con éxito.\n", nombre_usuario);
    } else {
        printf("Error al cambiar la contraseña para el usuario '%s'.\n", nombre_usuario);
    }
}








/*
 * Función principal
 * -----------------
 * Proporciona una terminal interactiva que permite al usuario ejecutar los comandos:
 *   - 'creadir <nombre>' para crear un directorio.
 *   - 'listar' para listar los directorios creados.
 *   - 'ir <nombre>' para cambiar al directorio especificado.
 *   - 'salir' para terminar el programa.
 *
 * Comportamiento:
 *   Muestra un prompt (`>`) para que el usuario ingrese comandos.
 *   El programa continúa ejecutándose hasta que el usuario escribe 'salir'.
 */
// Función principal


int main() {
    char comando[256];
    char *accion, *argumentos[10];  // Permitir hasta 10 argumentos (puedes ajustar este límite)
    int num_argumentos;

    printf("Bienvenido a la terminal personalizada. Escribe 'salir' para terminar.\n");

    while (1) {
        // Mostrar el prompt
        printf("> ");
        fgets(comando, sizeof(comando), stdin);

        // Limpieza del comando ingresado
        comando[strcspn(comando, "\n")] = 0;  // Eliminar salto de línea
        while (strlen(comando) > 0 && comando[strlen(comando) - 1] == ' ') {
            comando[strlen(comando) - 1] = '\0';  // Eliminar espacios al final
        }

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
                    printf("Error: No se proporcionó un nombre para el directorio.\n");
                }
            } else if (strcmp(accion, "listar") == 0) {
                listar_directorios(num_argumentos >= 1 ? argumentos[0] : NULL);
            } else if (strcmp(accion, "ir") == 0) {
                if (num_argumentos >= 1) {
                    cambiar_directorio(argumentos[0]);
                } else {
                    printf("Error: No se proporcionó un nombre para el directorio.\n");
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
                    copiar_archivo(argumentos[0], argumentos[1]);
                } else {
                    printf("Error: Debes proporcionar el archivo origen y el destino.\n");
                }
            } else if (strcmp(accion, "permisos") == 0) {
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
                if (num_argumentos >= 5) {
                    agregar_usuario(argumentos[0], argumentos[1], argumentos[2], argumentos[3], argumentos[4]);
                } else {
                    printf("Error: Debes proporcionar el nombre del usuario, contraseña, datos personales, horario y lugares de conexión.\n");
                }
            } else if (strcmp(accion, "contrasena") == 0) {
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

            else if (strcmp(accion, "salir") == 0) {
                break;
            } else {
                printf("Comando no reconocido: '%s'.\n", accion);
            }
        }
    }

    return 0;
}
