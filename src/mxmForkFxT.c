/*#######################################################################################
#* Fecha:
#* Autor: J. Corredor, PhD
#* Programa:
#*      Multiplicación de Matrices algoritmo clásico
#* Versión:
#*      Paralelismo con Procesos Fork 
######################################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "moduloMM.h" 

int main(int argc, char *argv[]) {
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);
	}

	int N		= (int) atoi(argv[1]);
	int num_P  	= (int) atoi(argv[2]);
    int filasxP	= N/num_P;

	double *matA = (double *) calloc(N*N, sizeof(double));
	double *matB = (double *) calloc(N*N, sizeof(double));
	double *matC = (double *) calloc(N*N, sizeof(double));
	double *matT = (double *) calloc(N*N, sizeof(double));

    iniMatrix(matA, matB, N);
    impMatrix(matA, N);
    impMatrix(matB, N);

	InicioMuestra();
	
	matrixTRP(N, matB, matT);

	for (int i = 0; i < num_P; i++) {
		pid_t pid = fork();
        
        if (pid == 0) { 
            int filaI = i*filasxP;
            int filaF = (i == num_P - 1) ? N : filaI + filasxP;
            
			mxmForkFxT(matA, matT, matC, N, filaI, filaF); 
            
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
    
    for (int i = 0; i < num_P; i++) {
        wait(NULL);
    }
  	
	FinMuestra(); 

 
	free(matA);
	free(matB);
	free(matC);
	free(matT);

    return 0;
}
