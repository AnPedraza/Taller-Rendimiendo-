/*#######################################################################################
#		Pontificia Universidad Javeriana
#* Fecha: 27/04/2026
#* Autor: David Pedraza, Johan Barreto, Oscar Pinilla
#* Programa:
#	mxmPosixFxC.c
#*      Multiplicación de Matrices algoritmo clásico
#* Versión:
#*      Paralelismo con Hilos Pthreads "Posix"
#	Descripción: Código para multiplicar matrices usando hilos, permitiendo que varios
#             núcleos del procesador trabajen al mismo tiempo para terminar más rápido. 
######################################################################################*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "moduloMM.h" 

/* Variables globales que todos los hilos pueden ver y usar. */
pthread_mutex_t MM_mutex;
double *matrixA, *matrixB, *matrixC;

/* Paquete de datos que le pasamos a cada hilo para que sepa qué parte le toca. */
struct parametros{
	int nH;
	int idH;
	int N;
};

/* Calcula un pedazo de la matriz. */
void *mxmPosixFxC(void *variables){
	struct parametros *data = (struct parametros *)variables;
	
	int idH		= data->idH;
	int nH		= data->nH;
	int D		= data->N;

/* Divide el total de filas entre los hilos disponibles para repartir el trabajo. */
	int filaI	= (D/nH)*idH;
	int filaF	= (D/nH)*(idH+1);

/* Recorre su parte asignada y hace la multiplicación fila por columna. */
    for (int i = filaI; i < filaF; i++){
        for (int j = 0; j < D; j++){
			double *pA, *pB, Suma = 0.0;
			pA = matrixA+i*D; 
			pB = matrixB+j;
            for (int k = 0; k < D; k++, pA++, pB+=D){
				Suma += *pA * *pB;
			}
			matrixC[i*D+j] = Suma;
		}
	}

/* El hilo terminó su trabajo. */
	pthread_mutex_lock (&MM_mutex);
	pthread_mutex_unlock (&MM_mutex);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
/* Revisa el tamaño de la matriz y cuántos hilos usar. */
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);	
	}
    int N		= (int) atoi(argv[1]); 
    int num_Th 	= (int) atoi(argv[2]); 

    pthread_t p[num_Th];
    pthread_attr_t atrMM;
/* Reserva el espacio en la memoria para guardar los números de las matrices. */
	matrixA  = (double *)calloc(N*N, sizeof(double));
	matrixB  = (double *)calloc(N*N, sizeof(double));
	matrixC  = (double *)calloc(N*N, sizeof(double));

/* Llena las matrices con números al azar y las muestra . */
	iniMatrix(matrixA, matrixB, N);
	impMatrix(matrixA, N);
	impMatrix(matrixB, N);
/* Empieza a contar el tiempo justo antes de lanzar los hilos. */
	InicioMuestra();

	pthread_mutex_init(&MM_mutex, NULL);
	pthread_attr_init(&atrMM);
	pthread_attr_setdetachstate(&atrMM, PTHREAD_CREATE_JOINABLE);

/* Crea cada hilo y le mete los datos. */
    for (int j=0; j<num_Th; j++){
		struct parametros *datos = (struct parametros *) malloc(sizeof(struct parametros)); 
		datos->idH = j;
		datos->nH  = num_Th;
		datos->N   = N;

        pthread_create(&p[j],&atrMM,mxmPosixFxC,(void *)datos);
	}

    for (int j=0; j<num_Th; j++)
        pthread_join(p[j],NULL);

/* Detiene el reloj y muestra cuánto tardó todo el proceso. */
	FinMuestra();
	
	impMatrix(matrixC, N);

	/*Liberación de Memoria*/
	free(matrixA);
	free(matrixB);
	free(matrixC);

	pthread_attr_destroy(&atrMM);
	pthread_mutex_destroy(&MM_mutex);
	pthread_exit (NULL);

	return 0;
}

