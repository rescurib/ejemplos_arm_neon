# Aceleración de código en C con arquitectura NEON de ARM

ARM NEON es el nombre de una extensión avanzada de arquitectura SIMD (Instrucción Única, Datos Múltiples) disponible para las familias ARM Cortex A y R.

> Los registros NEON se consideran vectores de elementos del mismo tipo de datos, y las instrucciones NEON operan sobre múltiples elementos simultáneamente. Esta tecnología admite diversos tipos de datos, incluyendo operaciones con números enteros y de punto flotante.

En otras palabras, esta arquitectura nos permite tener paralelismo a nivel de instrucción. Ya he hablado antes de como aprovechar el paralelismo a nivel de nucleos, pero en este ejemplo voy a mostrar como acelerar código únicamente a través de banderas de compilación.

Es importa saber el nombre del procesador que vamos a utilizar y el conjunto de instrucciones de arquitectura (ISA) que utiliza. En el caso de los últimos modelos de Rapberry Pi:

| Modelo de Raspberry Pi | CPU ARM | Conjunto de Instrucciones |
| ----- | ----- | ----- |
| **Raspberry Pi 3 Model B/B+** | Cortex-A53 | ARMv8-A (64/32-bit) |
| **Raspberry Pi 4 Model B** | Cortex-A72 | ARMv8-A (64/32-bit) |
| **Raspberry Pi 5** | Cortex-A76 | ARMv8.2-A (64/32-bit) |

## Ejemplo con Raspberry 3B+

El programa que usararémos calcula el producto punto entre dos vectores 4096 elementos *float* cada uno. El loop de cómputo es:
```C
for(j=0;j<ARRAY_SIZE;j++)
{
   result += array_a[j]*array_b[j];
}
```
Vamos a demostrar ahora que no basta con utilizar las [banderas](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html) `-o3`/`-Ofast` para que el compilador utilice las instrucciones vectoriales NEON. Para esto creamos un Makefile con dos targets: uno normal con la bandera `-o3` y otro con las [banderas especiales para procesadores ARM](https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html). 

>**NOTA**: Si no estas familiarizado con gcc o make, puedes revisar [esta](https://github.com/rescurib/Curso_Sistemas_Embebidos/tree/main/CursoSTM32/Lec_2/Practica_2_0) y [esta otra](https://github.com/rescurib/Curso_Sistemas_Embebidos/tree/main/CursoSTM32/Lec_2/Practica_2_1) práctica de mi curso de sistemas embebidos.

Las variables para los targets son estas:
```Makefile
# Optimización estandar
CFLAGS  := -O3

# Optimización NEON
NEON_CFLAGS := -O3 -march=armv8-a+simd -mtune=cortex-a53 \
               -ftree-vectorize -funsafe-math-optimizations \
               -fopt-info-vec-optimized
```

`-O3`
- Nivel de optimización más agresivo (ateniendo estándares) en GCC.  
- Activa:
  - **Inlining** más agresivo.  
  - **Loop unrolling** (desenrollado de bucles).  
  - **Vectorización automática** (cuando es posible).  
- Puede aumentar el tamaño del binario pero mejora la velocidad.

`-march=armv8-a+simd`
- Define la arquitectura: **ARMv8-A** (64 bits).  
- El `+simd` habilita las instrucciones **NEON/Advanced SIMD**.  
- Indica al compilador que genere instrucciones SIMD sin necesidad de *[intrínsecos](https://developer.arm.com/documentation/den0018/a/NEON-Intrinsics/Introduction?lang=en)* manuales cuando sea posible (hablaré del modo manual al final).

`-mtune=cortex-a53`
- Ajusta la **sintonía de optimización** a la microarquitectura **Cortex-A53**.  
- No cambia las instrucciones generadas (eso lo define `-march`),  
  sino el ordenamiento y planificación de instrucciones para aprovechar mejor el **pipeline** y las **cachés** del A53.

`-ftree-vectorize`
- Fuerza la **vectorización automática** de bucles, si el compilador detecta que es seguro.  
- Convierte operaciones escalares en operaciones SIMD.  
- Requiere un nivel de optimización alto (`-O3` lo activa por defecto, pero aquí se usa explícitamente).

`-funsafe-math-optimizations`
- Permite optimizaciones matemáticas que **no preservan exactamente la semántica IEEE 754**.  
- Ejemplos:
  - Reordenamiento de operaciones (no necesariamente asociativo en coma flotante).  
  - Sustituir divisiones por multiplicaciones con el recíproco.  
- Mejora mucho el rendimiento, pero puede introducir pequeños errores numéricos (hay que evaluar el resultado con cuidado y omitir esta bandera si es necesario).

`-fopt-info-vec-optimized`
- Indica al compilador que **muestre en la salida** qué bucles fueron efectivamente **vectorizados**.  
- Útil para verificar que la vectorización automática está funcionando. La salida para este ejemplo es:
```Bash
dot.c:30:18: optimized: loop vectorized using 16 byte vectors
``` 

### Compilación y evaluación de desempeño
```Bash
# Genera binario con optimización estandar
make

# Genera binario con optmización para ARM NEON
make neon

# Asignar permisos de ejecución
chmod +x performance.sh

# Ejecutar script de evalación
./performance.sh
```

## Ejecución en Raspberry Pi 3B+
```Bash
--- Ejecutando: ./o3_dot (x50 veces) ---
Promedio de tiempo de ejecución:
   43624.6517 nanosegundos

--- Ejecutando: ./neon_dot (x50 veces) ---
Promedio de tiempo de ejecución:
   15954.4456 nanosegundos

========================================
Mejora por Optimización NEON: 63.00%
========================================
```

## BONUS: Ejemplo de optimización manual usando *intrínsecos*

Los `intrínsecos` son funciones que están mapeados a una secuencia especifica de intrucciones (o incluso una sola, según el caso). Cuando el compilador los ve, no se la piensa demasiado, simplemente ya sabe que hacer. En el caso de NEON, son funciones de las que podemos estar seguros que se implementaran vectorialmente. Pueden encontrar la documentación de los intrínsicos de NEON [acá](https://developer.arm.com/documentation/dui0491/i/Using-NEON-Support/Intrinsics?lang=en)

**Producto punto con intrínsicos NEON**:
```C
#include <arm_neon.h>
void neon_mult_and_acc(const float* a, const float* b, float* result, int size)
{
    float32x4_t va, vb;
    float32x4_t vacc = vdupq_n_f32(0.0f);
    int i;
    for(i = 0; i<(size-4); i += 4)
    {
        va     = vld1q_f32(&a[i]);
        vb     = vld1q_f32(&b[i]);
        vacc   = vmlaq_f32(vacc,va,vb);
    }

    float32x2_t sum_vectors = vadd_f32(vget_low_f32(vacc),vget_high_f32(vacc));
    *result = vget_lane_f32(sum_vectors,0) + vget_lane_f32(sum_vectors,1);

    // Elementos restantes
    for(; i<size; i++)
    {
        *result += a[i]*b[i];
    }
}
```

| Intrinsic | Descripción | Tipo devuelto | Equivalente conceptual |
| --------- | ----------- | --------------|----------------------- |
| `vdupq_n_f32(x)`       | Crea un vector de 4 elementos (`float32x4_t`) donde **`x` es asignado a todos los elementos**. Se usa para inicializar el acumulador. | `float32x4_t` | `[x, x, x, x]`                       |
| `vld1q_f32(ptr)`       | Carga **4 valores consecutivos** desde la memoria apuntada por `ptr` en un registro NEON de 128 bits.                                         | `float32x4_t` | `[*ptr[0], *ptr[1], *ptr[2], *ptr[3]]`   |
| `vmlaq_f32(acc, a, b)` | Realiza una **operación vectorial de multiplicar y acumular**: `acc + (a * b)` para cada elemento del vector.                                 | `float32x4_t` | `for(i=0..3) acc[i] += a[i] * b[i];` |
| `vget_low_f32(v)`      | Extrae los **dos elementos inferiores** de un vector de 4 floats.                                                                             | `float32x2_t` | `[v[0], v[1]]`                       |
| `vget_high_f32(v)`     | Extrae los **dos elementos superiores** de un vector de 4 floats.                                                                             | `float32x2_t` | `[v[2], v[3]]`                       |
| `vadd_f32(a, b)`       | Suma dos vectores de 2 floats elemento a elemento.                                                                                            | `float32x2_t` | `[a[0]+b[0], a[1]+b[1]]`             |
| `vget_lane_f32(v, n)`  | Devuelve el valor del **elemento `n`** (0 o 1) del vector de 2 floats.                                                                        | `float`       | `v[n]`                               |


**Compilación**:
```Bash
gcc -O3 neon_intrinsics.c -o neon_manual
```

**Resultado:**
```Bash
./neon_manual
Resultado escalar: 1033.4281
Tiempo de ejecución escalar: 40207.2840 nanosegundos
Resultado NEON: 1033.4296
Tiempo de ejecución NEON: 19887 nanosegundos
Porcentaje de mejora: 202.18%
```

¡202%! Tenemos un claro método ganador. Si ejecután varias veces el programa notarán que hay una variación de entre 160 - 205%. El problema con este método es que requiere bastante tiempo de lectura de la documentación, elección de la intrincios adecuados y desarrollo del programa. Una alternativa más rápida es usar la librería [Ne10](https://projectne10.github.io/Ne10/) que provee de funciones matemáticas como filtros, FFT y operaciones vectoriales que ya están optmizadas con NEON.