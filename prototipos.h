#ifndef PROTOTIPOS_H
#define PROTOTIPOS_H

// Declaraciones de funciones
int es_comando_prohibido(const char *comando);
void ejecutar_comando(const char *comando);

void registrar_error(const char *mensaje);   // Registra los errores en el sistema que se le presentan al usuario
void registrar_movimientos(const char *comando); // Registra comandos con un timestamp en el archivo log

#endif // PROTOTIPOS_H
