#include <stdio.h>
#include <stddef.h>
#ifndef PROTOTIPOS_H
#define PROTOTIPOS_H


// Declaraciones de funciones
int es_comando_prohibido(const char *comando);
void ejecutar_comando(const char *comando);

void registrar_error(const char *mensaje);   // Registra los errores en el sistema que se le presentan al usuario
void registrar_movimientos(const char *comando); // Registra comandos con un timestamp en el archivo log

void obtener_timestamp(char *buffer, size_t buffer_size);
void obtener_ip_actual(char *ip_buffer, size_t buffer_size);
void registrar_sesion(const char *usuario, const char *accion, const char *ip_actual, const char *horario_actual);
void validar_inicio_sesion(const char *usuario, const char *ip_actual, const char *horario_actual, FILE *log_file);

#endif // PROTOTIPOS_H
