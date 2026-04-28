/*#######################################################################################
 #		Pontificia Universidad Javeriana 
 #Fecha: 27/04/2026 
 #* Autores: Oscar Pinilla, David Pedraza, Johan Barreto
 #* Modulo: 
 #       -moduloMM.c     
 #* Versión:
 #*	 	Concurrencia de Tareas: Paralelismo sobre Multiplicación de Matrices
 #* Descripción:
 #       - Implementación de funciones para la multiplicación de matrices (MxM) 
 #               optimizadas mediante el uso de transposición y manejo de punteros. 
 #               Incluye herramientas para la toma de tiempos y gestión de hilos POSIX.
######################################################################################*/

#ifndef __MODULOMM_H__
#define __MODULOMM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include "moduloMM.h"


/* Estructuras globales para la medición de tiempo de ejecución */
struct timeval inicio, fin;


//Imprime la matriz en consola si su dimensión es manejable (D < 13) con una dimensión de la matriz DxD
void impMatrix(double *matrix, int D){
	if(D < 13){
		printf("\n");
		for(int i=0; i<D*D; i++){
			if(i%D==0) printf("\n");
			printf("%.2f ", matrix[i]);
		}
		printf("\n**-----------------------------**\n");
	}
}



//Realiza la transposición de una matriz para optimizar el acceso a memoria (caché).
 //dimension del tamaño de la matriz, mB Matriz original, mT Matriz transpuesta resultante
void matrixTRP(int N, double *mB, double *mT){
	for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
			mT[i*N+j] = mB[j*N+i];
	impMatrix(mT, N);
}


/* Realiza la multiplicación de matrices aprovechando que la segunda matriz ya está 
 transpuesta, lo que permite recorrer ambas de forma en la memoria Fila x Fila. */
void mxmForkFxT(double *mA, double *mT, double *mC, int D, int filaI, int filaF){
    for (int i = filaI; i < filaF; i++)
        for (int j = 0; j < D; j++) {
			double *pA, *pB, Suma = 0.0;
			pA = mA+i*D;
			pB = mT+j*D;
            for (int k = 0; k < D; k++, pA++, pB++) 
				Suma += *pA * *pB;	
			mC[i*D+j] = Suma;
        }
}



/* Ejecuta la multiplicación de matrices estándar, saltando 
   entre filas en la segunda matriz gracias a los punteros. */
void mxmForkFxC(double *mA, double *mB, double *mC, int D, int filaI, int filaF){
    for (int i = filaI; i < filaF; i++)
        for (int j = 0; j < D; j++) {
			double *pA, *pB, Suma = 0.0;
			pA = mA+i*D;
			pB = mB+j;
            for (int k = 0; k < D; k++, pA++, pB+=D) 
				Suma += *pA * *pB;	
			mC[i*D+j] = Suma;
        }
}


/* Guarda el tiempo inicial del sistema antes de ejecutar un proceso. */
void InicioMuestra(){
	gettimeofday(&inicio, (void *)0);
}


/* Calcula la diferencia de tiempo y la imprime en pantalla. */
void FinMuestra(){
	gettimeofday(&fin, (void *)0);
	fin.tv_usec -= inicio.tv_usec;
	fin.tv_sec  -= inicio.tv_sec;
	double tiempo = (double) (fin.tv_sec*1000000 + fin.tv_usec); 
	printf("%9.0f \n", tiempo);
}



/* Llena las matrices m1 y m2 con valores aleatorio */
void iniMatrix(double *m1, double *m2, int D){
	srand(time(NULL));
	for(int i=0; i<D*D; i++, m1++, m2++){
		*m1 = (double)rand()/RAND_MAX*(5.0-1.0);	
		*m2 = (double)rand()/RAND_MAX*(9.0-2.0);	
	}
}

#endif
