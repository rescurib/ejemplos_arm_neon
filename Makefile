# Makefile for benchmarking o3_dot vs neon_dot

CC      := gcc
SRC     := neon_dot.c

# Optimización estandar
CFLAGS  := -O3

# Banderas de optimización NEON
NEON_CFLAGS := -O3 -march=armv8-a+simd -mtune=cortex-a53 \
               -ftree-vectorize -funsafe-math-optimizations \
               -fopt-info-vec-optimized

# Target por defecto: 
all: o3_dot

o3_dot: $(SRC)
	$(CC) $(CFLAGS) $< -o $@

# Target optmizado con NEON
neon_dot: $(SRC)
	$(CC) $(NEON_CFLAGS) $< -o $@

# Limpieza de archivos generados
.PHONY: clean
clean:
	rm -f o3_dot neon_dot
