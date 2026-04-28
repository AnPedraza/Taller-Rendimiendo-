/*#######################################################################################
#* Fecha:
#* Autor: J. Corredor, PhD
#* Programa:
#*      Multiplicación de Matrices algoritmo clásico
#* Versión:
#*      Paralelismo con Hilos Pthreads "Posix" 
######################################################################################*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "moduloMM.h" 

pthread_mutex_t MM_mutex;
double *matrixA, *matrixB, *matrixC;

struct parametros{
	int nH;
	int idH;
	int N;
};

void *mxmPosixFxC(void *variables){
	struct parametros *data = (struct parametros *)variables;
	
	int idH		= data->idH;
	int nH		= data->nH;
	int D		= data->N;
	int filaI	= (D/nH)*idH;
	int filaF	= (D/nH)*(idH+1);

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

	pthread_mutex_lock (&MM_mutex);
	pthread_mutex_unlock (&MM_mutex);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);	
	}
    int N		= (int) atoi(argv[1]); 
    int num_Th 	= (int) atoi(argv[2]); 

    pthread_t p[num_Th];
    pthread_attr_t atrMM;

	matrixA  = (double *)calloc(N*N, sizeof(double));
	matrixB  = (double *)calloc(N*N, sizeof(double));
	matrixC  = (double *)calloc(N*N, sizeof(double));

	iniMatrix(matrixA, matrixB, N);
	impMatrix(matrixA, N);
	impMatrix(matrixB, N);

	InicioMuestra();

	pthread_mutex_init(&MM_mutex, NULL);
	pthread_attr_init(&atrMM);
	pthread_attr_setdetachstate(&atrMM, PTHREAD_CREATE_JOINABLE);

    for (int j=0; j<num_Th; j++){
		struct parametros *datos = (struct parametros *) malloc(sizeof(struct parametros)); 
		datos->idH = j;
		datos->nH  = num_Th;
		datos->N   = N;

        pthread_create(&p[j],&atrMM,mxmPosixFxC,(void *)datos);
	}

    for (int j=0; j<num_Th; j++)
        pthread_join(p[j],NULL);

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

