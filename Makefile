GCC = gcc
CFLAGS = -lm -O3
FOPENMP = -fopenmp
POSIX = -lpthread
BIN = bin
SRC = src

modulo = moduloMM

PROGRAMAS = mxmPosixFxC mxmPosixFxT mxmForkFxC mxmForkFxT 

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
