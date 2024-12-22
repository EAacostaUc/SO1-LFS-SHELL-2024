#include "prototipos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lista de comandos prohibidos, si intenta usar estos comandos te dara error, son los comandos que hemos implementado...
const char *comandos_prohibidos[] = {
    "cp", "mv", "rename", "ls", "mkdir", "cd", "chmod", "chown", "passwd", "adduser", "useradd", "service", "systemctl"
};

// Función para verificar si un comando está prohibido
int es_comando_prohibido(const char *comando) {
    for (int i = 0; i < sizeof(comandos_prohibidos) / sizeof(comandos_prohibidos[0]); i++) {
        // Comparar si el comando comienza con un comando prohibido
        if (strncmp(comando, comandos_prohibidos[i], strlen(comandos_prohibidos[i])) == 0) {
            return 1;
        }
    }
    return 0;
}

// Función para ejecutar comandos del sistema
void ejecutar_comando(const char *comando) {
    // Verificar si el comando está prohibido
    if (es_comando_prohibido(comando)) {
        printf("Error: El comando '%s' está prohibido.\n", comando);
        return;
    }

    // Abrir un proceso para ejecutar el comando
    FILE *fp = popen(comando, "r");
    if (fp == NULL) {
        printf("Error: No se pudo ejecutar el comando '%s'.\n", comando);
        return;
    }

    // Leer y mostrar la salida del comando
    char buffer[1024];
    printf("Salida del comando '%s':\n", comando);
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    // Cerrar el proceso
    pclose(fp);
}