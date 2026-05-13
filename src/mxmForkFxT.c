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
#include <stdlib.h>    /* calloc(), free(), atoi(), exit() */
#include <unistd.h>    /* fork(), getpid() */
#include <sys/wait.h>  /* wait() */
#include "moduloMM.h"  /* iniMatrix, impMatrix, matrixTRP, InicioMuestra, FinMuestra, mxmForkFxT */

/* -----------------------------------------------------------------------
   Esta versión combina dos técnicas:
   1. Paralelismo por procesos (fork): igual que mxmForkFxC.
   2. Optimización de caché (transpuesta): igual que mxmPosixFxT.

   Es la versión Fork más rápida del experimento, ya que elimina los
   cache misses del acceso por columnas gracias a la transpuesta de B.

   NOTA sobre fork y memoria compartida:
   La transpuesta (matT) se calcula en el padre ANTES del fork.
   Cuando el padre hace fork(), cada hijo hereda matT ya calculada
   (gracias a Copy-On-Write). Los hijos solo leen matT → no se crea
   una copia real en memoria para matT (COW no se activa para lecturas).
   Esto hace que el costo extra de memoria de matT sea compartido eficientemente.
   ----------------------------------------------------------------------- */

int main(int argc, char *argv[]) {
	/* Validación de argumentos */
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);
	}

	int N      = (int) atoi(argv[1]);   /* Dimensión N de la matriz N×N */
	int num_P  = (int) atoi(argv[2]);   /* Número de procesos hijos */
	int filasxP = N / num_P;            /* Filas por proceso (asume divisibilidad exacta) */

	/* Reserva de las 4 matrices en el proceso padre.
	   matT es la transpuesta de matB, que se calcula antes de los fork().
	   Para N=3200: cada matriz ocupa ~81 MB → total ~324 MB. */
	double *matA = (double *) calloc(N*N, sizeof(double));
	double *matB = (double *) calloc(N*N, sizeof(double));
	double *matC = (double *) calloc(N*N, sizeof(double));
	double *matT = (double *) calloc(N*N, sizeof(double));   /* Transpuesta de B */

	/* Inicializa A y B con valores aleatorios */
    iniMatrix(matA, matB, N);
    impMatrix(matA, N);
    impMatrix(matB, N);

	/* INICIO DE MEDICIÓN: incluye la transposición y toda la ejecución paralela.
	   El costo de matrixTRP se suma al tiempo total, igual que en mxmPosixFxT,
	   para una comparación justa entre las cuatro estrategias. */
	InicioMuestra();
	
	/* Calcula la transpuesta de B en matT.
	   Esta operación es O(N²) y se ejecuta una sola vez en el padre.
	   Los hijos heredan matT ya calculada → no repiten este trabajo.
	   El beneficio en caché durante la multiplicación compensa este costo previo. */
	matrixTRP(N, matB, matT);

	/* Bucle de creación de procesos hijos */
	for (int i = 0; i < num_P; i++) {
		pid_t pid = fork();   /* Duplica el proceso actual */
        
        if (pid == 0) { 
            /* ---- CÓDIGO DEL PROCESO HIJO ---- */

			/* Calcula el rango de filas que le corresponde procesar */
            int filaI = i * filasxP;
            int filaF = (i == num_P - 1) ? N : filaI + filasxP;
            
			/* Ejecuta la multiplicación con transpuesta en su rango de filas.
			   Usa matT (transpuesta) en lugar de matB → acceso secuencial en memoria.
			   El hijo escribe en su copia local de matC (no visible para el padre). */
			mxmForkFxT(matA, matT, matC, N, filaI, filaF); 
    
			/* Impresión de validación solo para matrices pequeñas */
			if(N < 11){
           		printf("\nChild PID %d calculated rows %d to %d:\n", getpid(), filaI, filaF-1);
            	for (int f = filaI; f < filaF; f++) {
                	for (int c = 0; c < N; c++) {
                    	printf(" %.2f ", matC[N*f+c]);
                	}
                	printf("\n");
            	}
			}

            exit(0);   /* Hijo termina; el SO libera su memoria */

        } else if (pid < 0) {
            /* Error al crear el proceso hijo */
            perror("fork failed");
            exit(1);
        }
        /* Padre continúa el bucle para crear el siguiente hijo */
    }
    
	/* El padre espera a que todos los hijos terminen.
	   Sin este wait(), el padre llamaría a FinMuestra() antes de que los hijos
	   completen su trabajo → tiempo medido incorrecto (subestimado). */
    for (int i = 0; i < num_P; i++) {
        wait(NULL);
    }
  	
	/* FIN DE MEDICIÓN: imprime tiempo en µs → capturado por lanzador.pl */
	FinMuestra(); 

	/* Libera la memoria del padre, incluyendo matT */
	free(matA);
	free(matB);
	free(matC);
	free(matT);   /* Exclusivo de esta versión */

    return 0;
}
