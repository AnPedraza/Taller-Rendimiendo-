/*#######################################################################################
#*		Pontificia Universidad Javeriana 
#Fecha: 27/04/2026
#* Autor: David Pedraza, Oscar Pinilla, Johan Barreto
#* Programa:
#	mxmForkFxC.c
#*      Multiplicación de Matrices algoritmo clásico
#* Versión:
#*      Paralelismo con Procesos Fork
#	Descripción: Ejecución paralela del algoritmo clásico de multiplicación de matrices 
 #               distribuyendo la carga de trabajo entre procesos hijos independientes. 
######################################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "moduloMM.h" 

int main(int argc, char *argv[]) {

/* Valida que se ingresen los argumentos*/
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);
	}

/* Asigna el tamaño de la matriz, el número de procesos y calcula la carga de cada proceso. */
	int N		= (int) atoi(argv[1]);
	int num_P  	= (int) atoi(argv[2]);
    int filasxP	= N/num_P;


/* Reserva memoria dinámica para las matrices A, B y el resultado C. */
	double *matA = (double *) calloc(N*N, sizeof(double));
	double *matB = (double *) calloc(N*N, sizeof(double));
	double *matC = (double *) calloc(N*N, sizeof(double));


/* Inicializa y muestra las matrices de entrada segun la dimensión que tenga. */
    iniMatrix(matA, matB, N);
    impMatrix(matA, N);
    impMatrix(matB, N);

	InicioMuestra();


/* Crea hijos para repartir las filas de la matriz A. */
	for (int i = 0; i < num_P; i++) {
		pid_t pid = fork();
        
        if (pid == 0) { 
/* Define su rango de trabajo y ejecuta el cálculo asignado. */
            int filaI = i*filasxP;
            int filaF = (i == num_P - 1) ? N : filaI + filasxP;
            
			mxmForkFxC(matA, matB, matC, N, filaI, filaF); 

/* Si la matriz es <11, imprime el segmento calculado. */            
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

            perror("fork failed");
            exit(1);
        }
    }

/* Espera a que terminen su ejecución. */    
    for (int i = 0; i < num_P; i++) {
        wait(NULL);
    }
  	
	FinMuestra(); 

 /* Libera la memoria utilizada antes de finalizar. */
	free(matA);
	free(matB);
	free(matC);

    return 0;
}
