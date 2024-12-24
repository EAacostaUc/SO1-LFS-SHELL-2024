#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "prototipos.h"

// Función para simular un demonio
// Esta función simula un demonio escribiendo en un archivo de log y ejecutando un bucle infinito.
void fake_daemon() {
    // Abre un archivo de log para registrar la actividad del demonio.
    FILE *log = fopen("/tmp/fake_daemon.log", "a");
    if (!log) {
        perror("Error al abrir el archivo de log");
        exit(1);  // Si no se puede abrir el archivo, termina el programa con error.
    }

    // Registra que el demonio se ha iniciado.
    fprintf(log, "Fake daemon iniciado.\n");
    fflush(log);  // Asegura que el contenido se escriba inmediatamente en el archivo.

    // Bucle infinito para simular la actividad del demonio.
    while (1) {
        // Registra que el demonio está en ejecución.
        fprintf(log, "fake_daemon ejecutandose...\n");
        fflush(log);  // Asegura que se escriba el contenido en el archivo.
        sleep(3);  // El demonio duerme 3 segundos entre cada registro, simulando actividad.
    }

    fclose(log);  // Este código nunca se alcanzará debido al bucle infinito.
}

// Función para iniciar el demonio
// Esta función crea un nuevo proceso utilizando fork(), que ejecutará el demonio.
void iniciar_demonio(const char *demonio) {
    pid_t pid = fork();  // Crea un nuevo proceso.
    if (pid < 0) {
        perror("Error al hacer fork");  // Si ocurre un error al hacer fork, muestra el mensaje de error.
        exit(1);
    }

    if (pid == 0) {
        // Proceso hijo (el demonio).
        setsid();  // Crea una nueva sesión para que el proceso hijo sea independiente del terminal.

        // Si el demonio es 'fake_daemon', llama a la función fake_daemon().
        if (strcmp(demonio, "fake_daemon") == 0) {
            fake_daemon();
        } else {
            // Si el demonio no es 'fake_daemon', intenta ejecutar el demonio real.
            execlp(demonio, demonio, (char *)NULL);
            perror("Error al ejecutar el demonio");
            exit(1);  // Si execlp falla, termina el programa con error.
        }
    } else {
        // Proceso padre: imprime el PID del demonio y lo guarda en un archivo.
        printf("Demonio '%s' iniciado con éxito. PID: %d\n", demonio, pid);
        FILE *pid_file = fopen("/tmp/fake_daemon.pid", "w");
        if (pid_file) {
            fprintf(pid_file, "%d\n", pid);  // Guarda el PID generado por fork() en el archivo.
            fclose(pid_file);
        } else {
            perror("Error al guardar el PID");
        }
    }
}

// Obtener el PID del demonio desde el archivo
// Lee el PID del demonio desde un archivo para poder gestionar el proceso.
pid_t obtener_pid(const char *demonio) {
    FILE *pid_file = fopen("/tmp/fake_daemon.pid", "r");  // Abre el archivo donde se guarda el PID.
    if (!pid_file) {
        printf("No se encontro el archivo del PID para el demonio '%s'.\n", demonio);
        return -1;  // Si no se encuentra el archivo, retorna -1.
    }

    pid_t pid;
    if (fscanf(pid_file, "%d", &pid) != 1) {
        printf("Error al leer el PID del demonio '%s'.\n", demonio);
        fclose(pid_file);
        return -1;  // Si no se puede leer el PID correctamente, retorna -1.
    }

    fclose(pid_file);
    return pid;  // Retorna el PID leído desde el archivo.
}

// Función para detener el demonio
// Envia una señal SIGTERM al demonio para detenerlo.
void detener_demonio(const char *demonio) {
    pid_t pid = obtener_pid(demonio);  // Obtiene el PID del demonio desde el archivo.
    if (pid == -1) {
        // Si no se encuentra el PID, muestra un mensaje de error y registra el error en el log.
        printf("Error al intentar detener el demonio (servicio no inicializado) '%s'.\n", demonio);

        // Registrar en el log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al intentar detener el demonio (servicio no inicializado) '%s'.\n", demonio);
        registrar_error(mensaje);

        return;  
    }

    // Intenta detener el demonio enviando SIGTERM.
    if (kill(pid, SIGTERM) == 0) {
        printf("Demonio '%s' detenido con exito.\n", demonio);
    } else {
        printf("Error al intentar detener el demonio (servicio no inicializado)'%s'.\n", demonio);  // Si no se puede detener el demonio, muestra un error.

        // Registrar en el log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al intentar detener el demonio (servicio no inicializado) '%s'.\n", demonio);
        registrar_error(mensaje);
    }

    
}

// Función principal para gestionar demonios
// Esta función se encarga de iniciar o detener el demonio basado en la acción pasada...
void gestionar_demonio(const char *accion, const char *demonio) {
    // Si la acción es 'start' y el demonio es 'fake_daemon', se inicia el demonio.
    if (strcmp(accion, "start") == 0 && strcmp(demonio, "fake_daemon") == 0) {
        iniciar_demonio(demonio);
    } 
    // Si la acción es 'stop' y el demonio es 'fake_daemon', se detiene el demonio...
    else if (strcmp(accion, "stop") == 0 && strcmp(demonio, "fake_daemon") == 0) {
        detener_demonio(demonio);
    } 
    else {
        // Si la acción no es válida, muestra un mensaje de error.
        printf("Accion no valida. Usa 'start' o 'stop'.\n");

        // Registrar en el log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error, accion no valida. Usa 'start' o 'stop'.\n'%s'.\n", demonio);
        registrar_error(mensaje);
    }
}
