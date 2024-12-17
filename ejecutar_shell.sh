#!/bin/bash

# Verificar si el script se está ejecutando como root
if [ "$(id -u)" -ne 0 ]; then

    # Crear el directorio /var/log/shell si no existe
    sudo mkdir -p /var/log/shell

    # Crear los archivos de log si no existen
    sudo touch /var/log/shell/usuario_horarios_log.log
    sudo touch /var/log/shell/shell_movimientos.log
    sudo touch /var/log/shell/sistema_error.log

    # Asignar permisos 666 a los archivos de log para que puedan ser leídos y escritos por cualquier usuario
    sudo chmod 666 /var/log/shell/usuario_horarios_log.log
    sudo chmod 666 /var/log/shell/shell_movimientos.log
    sudo chmod 666 /var/log/shell/sistema_error.log

    echo "Archivos de log creados y permisos asignados correctamente."
else
    echo "Se está ejecutando como root, no es necesario crear directorios ni archivos."
fi


# Compilar el código fuente con gcc
gcc -o shell *.c

# Ejecutar tu shell personalizado
./shell

