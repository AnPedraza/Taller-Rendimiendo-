GCC = gcc
CFLAGS = -lm -O3
FOPENMP = -fopenmp
POSIX = -lpthread

# Definición de carpetas para organizar los archivos
BIN = bin
SRC = src

modulo = moduloMM

PROGRAMAS = mxmPosixFxC mxmPosixFxT mxmForkFxC mxmForkFxT 

#Evalúa cómo los hilos y procesos reparten tareas para multiplicar matrices de forma simultánea,
#usando el método tradicional contra uno que usa la transposición para que los hilos lean la memoria más rápido.
All: $(PROGRAMAS)

mxmPosixFxC:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ $(POSIX)

mxmForkFxC:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ 

mxmPosixFxT:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ $(POSIX)

mxmForkFxT:
	$(GCC) $(CFLAGS) $(SRC)/$(modulo).c $(SRC)/$@.c -o $(BIN)/$@ 

clean:
	$(RM) $(BIN)/*
