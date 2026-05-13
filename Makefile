
#**************************************************************
#         		Pontificia Universidad Javeriana
#     Autor: J. Corredor 
#	  Nombre Estudiante: David Pedraza
#     Fecha:11 de mayo de 2026
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     File: Makefile
#****************************************************************/

# ============================================================
# Compilador y flags de compilación
# ============================================================

# Compilador a usar: gcc (GNU C Compiler)
GCC = gcc

# Flags de compilación comunes a todos los programas:
#   -lm:  enlaza la biblioteca matemática (math.h). Necesario para funciones
#         como sqrt(), pow(), etc. aunque en este proyecto no se usen directamente,
#         es buena práctica incluirla en proyectos numéricos.
#   -O3:  nivel de optimización máximo del compilador. GCC aplica transformaciones
#         agresivas: vectorización, inlining, unrolling de bucles, etc.
#         IMPORTANTE: -O3 puede hacer que la diferencia FxC vs FxT sea menos
#         pronunciada, ya que el compilador puede reordenar accesos a memoria
#         automáticamente. Para medir el efecto puro de la transpuesta se usaría -O0.
CFLAGS = -lm -O3

# Flag para OpenMP (paralelismo con directivas #pragma omp).
# No se usa en ninguno de los 4 programas actuales (todos usan Pthreads o Fork),
# pero está definido por si se agrega una variante con OpenMP en el futuro.
FOPENMP = -fopenmp

# Flag para enlazar la biblioteca POSIX Threads (Pthreads).
# Necesario en tiempo de enlazado para los programas que usan pthread_create,
# pthread_join, pthread_mutex_*, etc.
# Solo se añade a los targets que lo necesitan (mxmPosixFxC y mxmPosixFxT).
POSIX = -lpthread

# ============================================================
# Estructura de directorios del proyecto
# ============================================================

# Carpeta donde se depositan los ejecutables compilados.
# El lanzador.pl busca los binarios en bin/ con la ruta $Path/bin/$nombre
BIN = bin

# Carpeta donde están los archivos fuente .c y .h
SRC = src

# ============================================================
# Módulo compartido
# ============================================================

# Nombre base del módulo de utilidades compartido por todos los programas.
# Se expande a src/moduloMM.c en cada regla de compilación.
# Al compilarlo junto con cada ejecutable (en lugar de como .o separado),
# se evita el paso explícito de generación de objeto intermedio.
modulo = moduloMM

# ============================================================
# Lista de todos los programas a compilar
# ============================================================

# Los 4 targets que se generarán al ejecutar 'make' o 'make All'.
# Cada nombre coincide exactamente con el nombre del archivo .c en src/
# y con el nombre del ejecutable que se deposita en bin/.
PROGRAMAS = mxmPosixFxC mxmPosixFxT mxmForkFxC mxmForkFxT 

# ============================================================
# Target principal
# ============================================================

# 'All' es el target por defecto (el primero del Makefile → se ejecuta con 'make').
# Depende de todos los programas: make los compila en orden si no existen o están desactualizados.
# NOTA: la convención estándar en Makefile es 'all' en minúsculas; 'All' con mayúscula
# funciona igual pero no es el nombre estándar de POSIX.
All: $(PROGRAMAS)

# ============================================================
# Reglas de compilación de cada ejecutable
# ============================================================

# mxmPosixFxC: Pthreads + acceso por columnas (cache-unfriendly)
# Compila moduloMM.c junto con mxmPosixFxC.c y enlaza con -lpthread.
# $@ es una variable automática de make que se expande al nombre del target actual
# → en este caso "mxmPosixFxC", por lo que $(SRC)/$@.c = src/mxmPosixFxC.c
#   y $(BIN)/$@ = bin/mxmPosixFxC
mxmPosixFxC:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ $(POSIX)

# mxmForkFxC: Fork + acceso por columnas (cache-unfriendly)
# No necesita -lpthread porque usa fork() y wait() de <unistd.h>/<sys/wait.h>,
# que forman parte de la biblioteca estándar de C (libc), ya enlazada por defecto.
mxmForkFxC:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ 

# mxmPosixFxT: Pthreads + transpuesta (cache-friendly)
# Igual que mxmPosixFxC pero el código fuente usa matrixTRP antes de multiplicar.
# Requiere -lpthread por el uso de Pthreads.
mxmPosixFxT:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ $(POSIX)

# mxmForkFxT: Fork + transpuesta (cache-friendly)
# Versión Fork más rápida del experimento.
# No necesita -lpthread; fork() es una syscall POSIX incluida en libc.
mxmForkFxT:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ 

# ============================================================
# Target de limpieza
# ============================================================

# 'make clean' elimina todos los archivos dentro de bin/.
# $(RM) se expande a 'rm -f' por defecto en GNU Make (no falla si no existen archivos).
# NOTA: no elimina los archivos .dat de Soluciones/ ni los fuentes de src/,
# solo los binarios compilados.
clean:
	$(RM) $(BIN)/*