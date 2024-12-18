#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "prototipos.h"

// Iniciar un demonio
void iniciar_demonio(const char *demonio) {
    char comando[256];
    snprintf(comando, sizeof(comando), "nohup /usr/sbin/%s &", demonio);  // Usamos nohup para iniciar el demonio en segundo plano
    int resultado = system(comando);

    if (resultado == 0) {
        printf("Demonio '%s' iniciado con éxito.\n", demonio);
    } else {
        printf("Error: No se pudo iniciar el demonio '%s'.\n", demonio);

        
    }
}

// Detener el demonio
void detener_demonio(const char *demonio) {
    char comando[256];

    // Usamos killall para matar todos los procesos con el nombre del demonio
    snprintf(comando, sizeof(comando), "killall -9 %s", demonio);

    // Ejecutar el comando
    int resultado = system(comando);
    if (resultado == 0) {
        printf("Demonio '%s' detenido con éxito.\n", demonio);
    } else {
        printf("Error al intentar detener el demonio (servicio no inicializado)'%s'.\n", demonio);

        // Registrar en el log
        char mensaje[256];
        snprintf(mensaje, sizeof(mensaje), "Error al intentar detener el demonio (servicio no inicializado)'%s'.\n", demonio);
        registrar_error(mensaje);
    }
}

// Función principal para gestionar demonios
void gestionar_demonio(const char *accion, const char *demonio) {
    if (strcmp(accion, "start") == 0) {
        iniciar_demonio(demonio);
    } else if (strcmp(accion, "stop") == 0) {
        detener_demonio(demonio);
    } else {
        printf("Acción no válida. Usa 'start' o 'stop'.\n");
    }
}