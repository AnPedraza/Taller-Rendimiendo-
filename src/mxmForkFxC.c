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
#include <stdlib.h>    /* calloc(), free(), atoi(), exit() */
#include <unistd.h>    /* fork(), getpid() */
#include <sys/wait.h>  /* wait(): para que el padre espere a los hijos */
#include "moduloMM.h"  /* iniMatrix, impMatrix, InicioMuestra, FinMuestra, mxmForkFxC */

/* -----------------------------------------------------------------------
   DIFERENCIA FUNDAMENTAL RESPECTO A LOS PROGRAMAS POSIX:
   - Pthreads: múltiples hilos dentro de UN SOLO proceso → memoria compartida.
     Todos los hilos leen y escriben las mismas matrices.
   - Fork: se crean PROCESOS HIJOS separados → cada uno tiene su PROPIA COPIA
     del espacio de memoria (gracias a Copy-On-Write del SO).
     Los hijos calculan su parte de C en su copia local, pero esa copia
     NO es visible para el padre ni para los otros hijos.
     El resultado en matrixC del padre queda sin modificar.
   Este es el motivo por el que la implementación con fork es conceptualmente
   incorrecta para producir un resultado unificado, aunque es válida
   para medir el tiempo de cómputo paralelo.
   ----------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

	/* Validación de argumentos: se requiere tamaño de matriz y número de procesos */
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);
	}

	int N      = (int) atoi(argv[1]);   /* Dimensión N de la matriz N×N */
	int num_P  = (int) atoi(argv[2]);   /* Número de procesos hijos a crear */
	int filasxP = N / num_P;            /* Filas que le corresponden a cada proceso.
                                           Asume que N es divisible exactamente por num_P.
                                           Con los valores del lanzador (200,800,1600,3200
                                           y 1,2,4 procesos) esto siempre se cumple. */

	/* Reserva de memoria para las tres matrices en el proceso padre.
	   Cuando se hace fork(), cada hijo recibe una copia de estas matrices
	   gracias al mecanismo Copy-On-Write del kernel. */
	double *matA = (double *) calloc(N*N, sizeof(double));
	double *matB = (double *) calloc(N*N, sizeof(double));
	double *matC = (double *) calloc(N*N, sizeof(double));

	/* Inicializa A y B con valores aleatorios en el proceso padre.
	   Al hacer fork, los hijos heredan estos datos inicializados. */
    iniMatrix(matA, matB, N);
    impMatrix(matA, N);
    impMatrix(matB, N);

	/* INICIO DE MEDICIÓN: se mide desde aquí hasta FinMuestra() en el padre.
	   El tiempo incluye: la creación de procesos (fork), la ejecución paralela
	   de los hijos y la espera (wait). No incluye iniMatrix ni calloc. */
	InicioMuestra();

	/* Bucle de creación de procesos hijos */
	for (int i = 0; i < num_P; i++) {
		pid_t pid = fork();   /* Crea un proceso hijo; retorna:
                                 - 0 si estamos en el proceso hijo
                                 - PID del hijo si estamos en el padre
                                 - -1 si ocurrió un error */
        
        if (pid == 0) {
            /* ---- CÓDIGO DEL PROCESO HIJO ---- */

			/* Cada hijo calcula su rango de filas:
			   - i es el índice del hijo (0..num_P-1)
			   - El último hijo puede tener filas extra si N no es divisible exactamente,
			     pero el operador ternario lo maneja: si es el último, llega hasta N.
			     (En la práctica, con los tamaños del lanzador siempre hay divisibilidad exacta) */
            int filaI = i * filasxP;
            int filaF = (i == num_P - 1) ? N : filaI + filasxP;
            
			/* Llama a la función de multiplicación clásica fila×columna.
			   El hijo escribe el resultado en su copia local de matC.
			   IMPORTANTE: esta escritura NO es visible para el padre ni otros hijos
			   porque cada proceso tiene su propio espacio de memoria tras el fork. */
			mxmForkFxC(matA, matB, matC, N, filaI, filaF); 

			/* Imprime el resultado parcial solo si la matriz es pequeña (N<11).
			   Útil para validación visual durante el desarrollo. */
			if(N < 11){
           		printf("\nChild PID %d calculated rows %d to %d:\n", getpid(), filaI, filaF-1);
            	for (int f = filaI; f < filaF; f++) {
                	for (int c = 0; c < N; c++) {
                    	printf(" %.2f ", matC[N*f+c]);
                	}
                	printf("\n");
            	}
			}

            exit(0);   /* El hijo termina con código 0 (éxito).
                          Sus recursos (memoria, descriptores) son liberados por el SO. */

        } else if (pid < 0) {
            /* Error al crear el proceso hijo: se informa y se termina el padre */
            perror("fork failed");
            exit(1);
        }
        /* Si pid > 0: estamos en el padre, continuamos el bucle para crear el siguiente hijo */
    }

	/* El padre espera a que TODOS los hijos terminen.
	   wait(NULL) bloquea hasta que cualquier hijo termine y descarta su estado de salida.
	   Se llama num_P veces para esperar a cada uno de los hijos.
	   Sin este bucle, el padre podría llamar a FinMuestra() antes de que
	   los hijos terminen, midiendo un tiempo incorrecto (muy corto). */
    for (int i = 0; i < num_P; i++) {
        wait(NULL);
    }
  	
	/* FIN DE MEDICIÓN: imprime el tiempo transcurrido en microsegundos.
	   Este valor es capturado por lanzador.pl y guardado en el .dat. */
	FinMuestra(); 

	/* Libera la memoria del proceso padre.
	   La memoria de los hijos ya fue liberada por el SO al hacer exit(0). */
	free(matA);
	free(matB);
	free(matC);

    return 0;
}
