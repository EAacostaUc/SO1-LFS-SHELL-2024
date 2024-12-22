// estas cabeceras fueron necesarias para el funcionamiento correcto (generaba error al no incluirlas)
#include <stdio.h>  // para trabajar con FILE
#include <stddef.h>  //para trabajar con size_t
#ifndef PROTOTIPOS_H
#define PROTOTIPOS_H


// Declaraciones de funciones
int es_comando_prohibido(const char *comando);
void ejecutar_comando(const char *comando);

void registrar_error(const char *mensaje);   // Registra los errores en el sistema que se le presentan al usuario
void registrar_movimientos(const char *comando); // Registra comandos con un timestamp en el archivo log
void obtener_timestamp(char *buffer, size_t buffer_size);
void obtener_ip_actual(char *ip_buffer, size_t buffer_size);
void validar_inicio_sesion(const char *usuario, const char *ip_actual, const char *hora_entrada, const char *hora_salida, FILE *log_file, int es_salida);
void registrar_sesion(const char *usuario, const char *accion, const char *ip_actual, const char *hora_actual, int es_salida);

// Prototipo de la función gestionar_demonio
void gestionar_demonio(const char *accion, const char *demonio);


// Prototipos de funciones para Scp
void ejecutar_transferencia_scp(const char *archivo_local, const char *destino);
void registrar_transferencia_log(const char *archivo_local, const char *destino, int exito);

#endif // PROTOTIPOS_H
