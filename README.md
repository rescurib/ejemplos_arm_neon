# ejemplos_arm_neon
Ejemplos sobre uso de banderas de compilación en gcc para aprovechamiento de la arquitectura NEON de ARM.

## Ejecución en Raspberry Pi 3B+
```Bash
--- Ejecutando: ./o3_dot (x50 veces) ---
Promedio de tiempo de ejecución:
   41289.3013 microsegundos

--- Ejecutando: ./neon_dot (x50 veces) ---
Promedio de tiempo de ejecución:
   15756.3316 microsegundos

========================================
Mejora por Optimización NEON: 61.00%
========================================
```
