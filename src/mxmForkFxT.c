/*#######################################################################################
#		Pontificia Universidad Javeriana
#* Fecha: 27/04/2026
#* Autor: Oscar Pinilla, Johan Barreto, David Pedraza
#* Programa:
#	mxmForkFxT.c
#*      Multiplicación de Matrices algoritmo clásico
#* Versión:
#*      Paralelismo con Procesos Fork 
#Descripción: Ejecución paralela de multiplicación de matrices utilizando una matriz 
#               transpuesta para mejorar el rendimiento mediante acceso a la memoria.
######################################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "moduloMM.h" 

int main(int argc, char *argv[]) {
/* Verifica el tamaño de matriz y número de procesos. */
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);
	}

/* Define dimensiones y calcula la distribución de filas. */
	int N		= (int) atoi(argv[1]);
	int num_P  	= (int) atoi(argv[2]);
    int filasxP	= N/num_P;

/* Reserva de memoria para matrices de entrada, resultado y la matriz auxiliar transpuesta. */
	double *matA = (double *) calloc(N*N, sizeof(double));
	double *matB = (double *) calloc(N*N, sizeof(double));
	double *matC = (double *) calloc(N*N, sizeof(double));
	double *matT = (double *) calloc(N*N, sizeof(double));

/* Inicializa datos aleatorios e inicia la muestra. */
    iniMatrix(matA, matB, N);
    impMatrix(matA, N);
    impMatrix(matB, N);

	InicioMuestra();
	
/* Genera la transpuesta de la matriz B. */
	matrixTRP(N, matB, matT);

/* Creación de varios procesos para ejecutar al mismo tiempo el cálculo de la matriz resultante. */
	for (int i = 0; i < num_P; i++) {
		pid_t pid = fork();
        
        if (pid == 0) { 


 /* Cada proceso hijo se encarga de calcular un conjunto de filas usando la optimización fila por fila. */
           int filaI = i*filasxP;
            int filaF = (i == num_P - 1) ? N : filaI + filasxP;
            
			mxmForkFxT(matA, matT, matC, N, filaI, filaF); 
    
/* Impresión de resultados parciales cuando las matrices son de tamaño <11*/        
			if(N<11){
           		printf("\nChild PID %d calculated rows %d to %d:\n", getpid(), filaI, filaF-1);
            	for (int f = filaI; f < filaF; f++) {
                	for (int c = 0; c < N; c++) {
                    	printf(" %.2f ", matC[N*f+c]);
                	}
                	printf("\n");
            	}
			}

            exit(0); 
        } else if (pid < 0) {
/*Controla los errores de generacion */
            perror("fork failed");
            exit(1);
        }
    }
    
/* El proceso principal espera a que todos los procesos terminen la ejecución paralela. */
    for (int i = 0; i < num_P; i++) {
        wait(NULL);
    }
  	
	FinMuestra(); 

/* Libera la memoria asignada */ 
	free(matA);
	free(matB);
	free(matC);
	free(matT);

    return 0;
}
