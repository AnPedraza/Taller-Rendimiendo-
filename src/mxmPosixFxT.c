/*#######################################################################################
#			Pontificia Universidad Javeriana
#* Fecha: 27/04/2026
#* Autor: Oscar Pinilla, David Pedraza Johan Barreto
#* Programa:
#	mxmPosixFxT.c
#*      Multiplicación de Matrices algoritmo clásico
#* Versión:
#*      Paralelismo con Hilos Pthreads "Posix" 
#	Descripción: Multiplicación de matrices usando hilos y una matriz transpuesta.
#               Esta combinación ayuda a que el computador procese los datos de forma
#               más eficiente al leer la memoria de manera lineal.
######################################################################################*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "moduloMM.h" 

/* Variables compartidas, las matrices y la libreria de pthread para sincronizar los hilos. */
pthread_mutex_t MM_mutex;
double *matrixA, *matrixB, *matrixT, *matrixC; 

/* Valor de identificación de los hilos y su uso */
struct parametros{
        int nH;
        int idH;
        int N;
};

/* Función que ejecuta cada hilo para multiplicar su parte de la matriz. */
void *mxmPosixFxT(void *variables){
	struct parametros *data = (struct parametros *)variables;
	int idH		= data->idH;
	int nH		= data->nH;
	int D		= data->N;

/* Calcula desde qué fila hasta qué fila le toca trabajar al hilo. */
	int filaI	= (D/nH)*idH;
	int filaF	= (D/nH)*(idH+1);

/* Realiza el cálculo usando la matriz transpuesta */
    for (int i = filaI; i < filaF; i++){
        for (int j = 0; j < D; j++){
			double *pA, *pB, Suma = 0.0;
			pA = matrixA + i*D; 
			pB = matrixT + j*D;

/* Al estar transpuesta se puede avanzar en ambas matrices parte por parte */
            for (int k = 0; k < D; k++, pA++, pB++){
				Suma += *pA * *pB;
			}
			matrixC[i*D+j] = Suma;
		}
	}

/* Indica que el hilo ha terminado */
	pthread_mutex_lock (&MM_mutex);
	pthread_mutex_unlock (&MM_mutex);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){

/* Revisa que se hayan pasado los datos de tamaño y cantidad de hilos por consola. */
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);	
	}
    int N 		= (int) atoi(argv[1]); 
    int num_Th	= (int) atoi(argv[2]); 

    pthread_t p[num_Th];
    pthread_attr_t atrMM;

/* Reserva el espacio necesario en la memoria para todas las matrices. */
	matrixA  = (double *)calloc(N*N, sizeof(double));
	matrixB  = (double *)calloc(N*N, sizeof(double));
	matrixC  = (double *)calloc(N*N, sizeof(double));
	matrixT  = (double *)calloc(N*N, sizeof(double));

/* Llena las matrices con números y las muestra */
	iniMatrix(matrixA, matrixB, N);
	impMatrix(matrixA, N);
	impMatrix(matrixB, N);

/* Empieza a medir el tiempo justo antes de preparar la transpuesta. */
	InicioMuestra();

/* Crea la versión transpuesta de la matriz B. */
	matrixTRP(N, matrixB, matrixT);

/* Prepara las herramientas de control para los hilos. */
	pthread_mutex_init(&MM_mutex, NULL);
	pthread_attr_init(&atrMM);
	pthread_attr_setdetachstate(&atrMM, PTHREAD_CREATE_JOINABLE);

/* Crea los hilos y les asigna su parte de trabajo. */
    for (int j=0; j<num_Th; j++){
		struct parametros *datos = (struct parametros *) malloc(sizeof(struct parametros)); 
		datos->idH = j;
		datos->nH  = num_Th;
		datos->N   = N;

        pthread_create(&p[j],&atrMM,mxmPosixFxT,(void *)datos);
	}

/* Espera a que todos los hilos terminen sus cálculos. */
    for (int j=0; j<num_Th; j++)
        pthread_join(p[j],NULL);

/* Termina de medir el tiempo y muestra el resultado final. */
	FinMuestra();
	
	impMatrix(matrixC, N);

	/*Liberación de Memoria*/
	free(matrixA);
	free(matrixB);
	free(matrixC);
	free(matrixT);

	pthread_attr_destroy(&atrMM);
	pthread_mutex_destroy(&MM_mutex);
	pthread_exit (NULL);

	return 0;
}

