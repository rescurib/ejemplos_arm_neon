#!/bin/bash

# --- Configuración ---
# Número de veces que se ejecutará cada binario para promediar
NUM_RUNS=50
# Archivos binarios a probar
BINARIES=("./o3_dot" "./neon_dot")

# Variables para almacenar los tiempos promedio
TIME_STANDARD=0.0
TIME_NEON=0.0

# ---------------------

# Función para ejecutar un binario y calcular el tiempo promedio
run_and_average() {
    local binary=$1
    local total_time=0.0
    local run_count=0
    local time_value

    echo "--- Ejecutando: $binary (x$NUM_RUNS veces) ---"

    # Bucle para ejecutar el binario NUM_RUNS veces
    for i in $(seq 1 $NUM_RUNS); do
        # Ejecutar el binario y capturar la salida
        output=$($binary)

        # Parsear el tiempo de ejecución usando grep y awk
        # Busca la línea "Tiempo de ejecución: X.XXXX microsegundos"
        # y extrae el valor numérico.
        time_value=$(echo "$output" | grep "Tiempo de ejecución:" | awk '{print $4}')

        # Verificar si el valor es un número flotante válido
        if [[ "$time_value" =~ ^[0-9]*\.[0-9]+$ ]]; then
            # Sumar al total
            total_time=$(echo "$total_time + $time_value" | bc -l)
            run_count=$((run_count + 1))
        else
            echo "ADVERTENCIA: No se pudo parsear el tiempo de ejecución en la ejecución $i."
        fi
    done

    # Calcular el promedio
    if [ "$run_count" -gt 0 ]; then
        # bc -l permite la aritmética de punto flotante
        average_time=$(echo "scale=4; $total_time / $run_count" | bc -l)
        echo "Promedio de tiempo de ejecución ($run_count ejecuciones):"
        echo "   $average_time microsegundos"
    else
        echo "Error: No se pudo obtener ningún tiempo de ejecución válido."
    fi

    echo
}

# --- Ejecución principal ---

# 1. Asegurarse de que los binarios existen
for bin in "${BINARIES[@]}"; do
    if [ ! -f "$bin" ]; then
        echo "Error: El binario $bin no se encuentra. ¡Ejecuta 'make' primero!"
        exit 1
    fi
done

# 2. Ejecutar la función para cada binario
for bin in "${BINARIES[@]}"; do
    run_and_average "$bin"
done
