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

/* NOTA: estas guardas (#ifndef/#define/#endif) son propias de archivos .h.
   Ponerlas en un .c no causa error (el .c solo se compila una vez), pero es
   una práctica incorrecta. Si otro .c hiciera #include "moduloMM.c" por error,
   la guarda impediría la doble definición, pero lo correcto es nunca incluir .c. */
#ifndef __MODULOMM_H__
#define __MODULOMM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include "moduloMM.h"   /* Incluye los prototipos definidos en el header */


/* Variables globales para medir el tiempo de ejecución.
   'inicio' se llena en InicioMuestra() y 'fin' en FinMuestra().
   Al ser globales, ambas funciones pueden acceder a ellas sin pasarlas por parámetro. */
struct timeval inicio, fin;


/* -----------------------------------------------------------------------
   impMatrix: imprime la matriz en consola si su tamaño es manejable.
   - matrix: puntero al arreglo lineal que representa la matriz D×D.
   - D: dimensión de la matriz.
   El umbral D < 13 evita imprimir matrices grandes (ej. 3200×3200)
   que harían ilegible la salida y enlentecerían innecesariamente el benchmark.
   La condición i%D==0 detecta el inicio de cada fila para insertar un salto de línea.
   ----------------------------------------------------------------------- */
void impMatrix(double *matrix, int D){
	if(D < 13){         /* Solo imprime matrices pequeñas */
		printf("\n");
		for(int i=0; i<D*D; i++){
			if(i%D==0) printf("\n");          /* Salto de línea al inicio de cada fila */
			printf("%.2f ", matrix[i]);       /* Imprime con 2 decimales */
		}
		printf("\n**-----------------------------**\n");
	}
}


/* -----------------------------------------------------------------------
   matrixTRP: calcula la transpuesta de mB y la guarda en mT.
   - N: dimensión de la matriz cuadrada N×N.
   - mB: matriz original (almacenada en row-major: elemento [i][j] = mB[i*N+j]).
   - mT: matriz resultado donde se guarda la transpuesta.

   La transposición intercambia filas y columnas: mT[i][j] = mB[j][i].
   En términos de índices lineales: mT[i*N+j] = mB[j*N+i].

   ¿Por qué transponemos?
   Al multiplicar A×B, el bucle interno accede a B por columnas (saltos de N posiciones).
   Si transponemos B→mT, ese mismo acceso se convierte en acceso por filas (secuencial),
   lo que reduce dramáticamente los cache misses, especialmente para matrices grandes.
   ----------------------------------------------------------------------- */
void matrixTRP(int N, double *mB, double *mT){
	for(int i=0; i<N; i++)
		for(int j=0; j<N; j++)
			mT[i*N+j] = mB[j*N+i];   /* Intercambia índices para transponer */
	impMatrix(mT, N);                 /* Muestra la transpuesta si N < 13 */
}


/* -----------------------------------------------------------------------
   mxmForkFxT: multiplicación de matrices usando la transpuesta de B.
   - mA: matriz A.
   - mT: transpuesta de B (calculada previamente con matrixTRP).
   - mC: matriz resultado C = A × B.
   - D: dimensión de las matrices cuadradas D×D.
   - filaI, filaF: rango de filas de A que procesa este llamador (proceso/hilo).

   Gracias a la transpuesta, el puntero pB recorre mT también de forma secuencial
   (avanza de 1 en 1), igual que pA. Ambos accesos son lineales en memoria →
   el prefetcher de la CPU puede cargar líneas de caché de forma anticipada,
   minimizando la latencia de acceso a RAM para matrices grandes.
   ----------------------------------------------------------------------- */
void mxmForkFxT(double *mA, double *mT, double *mC, int D, int filaI, int filaF){
    for (int i = filaI; i < filaF; i++)       /* Recorre las filas asignadas de A */
        for (int j = 0; j < D; j++) {          /* Para cada columna del resultado */
			double *pA, *pB, Suma = 0.0;
			pA = mA + i*D;    /* pA apunta al inicio de la fila i de A */
			pB = mT + j*D;    /* pB apunta al inicio de la fila j de mT
                                 (que es la columna j de B original) */
            for (int k = 0; k < D; k++, pA++, pB++) 
				/* Ambos punteros avanzan de 1 en 1 → acceso secuencial → cache-friendly */
				Suma += *pA * *pB;	
			mC[i*D+j] = Suma;  /* Guarda el producto punto en la posición [i][j] de C */
        }
}


/* -----------------------------------------------------------------------
   mxmForkFxC: multiplicación clásica fila por columna sin transponer.
   - mA: matriz A.
   - mB: matriz B sin modificar.
   - mC: matriz resultado C = A × B.
   - D: dimensión de las matrices cuadradas D×D.
   - filaI, filaF: rango de filas de A que procesa este llamador.

   El puntero pA avanza de 1 en 1 (secuencial, cache-friendly).
   El puntero pB avanza de D en D para recorrer una columna de B.
   Para D=3200 eso equivale a saltar 3200×8 = 25,600 bytes por iteración,
   lo que provoca un cache miss casi en cada acceso → rendimiento degradado.
   Esta variante sirve como línea base (baseline) del experimento de rendimiento.
   ----------------------------------------------------------------------- */
void mxmForkFxC(double *mA, double *mB, double *mC, int D, int filaI, int filaF){
    for (int i = filaI; i < filaF; i++)       /* Recorre las filas asignadas de A */
        for (int j = 0; j < D; j++) {          /* Para cada columna del resultado */
			double *pA, *pB, Suma = 0.0;
			pA = mA + i*D;  /* pA apunta al inicio de la fila i de A */
			pB = mB + j;    /* pB apunta al primer elemento de la columna j de B */
            for (int k = 0; k < D; k++, pA++, pB+=D) 
				/* pA avanza de 1 en 1 (OK), pB avanza de D en D → cache-unfriendly */
				Suma += *pA * *pB;	
			mC[i*D+j] = Suma;  /* Guarda el resultado en [i][j] de C */
        }
}


/* -----------------------------------------------------------------------
   InicioMuestra: registra el instante de inicio para medir tiempo.
   Usa gettimeofday() que ofrece resolución de microsegundos.
   El segundo argumento (zona horaria) se pasa como NULL porque no se necesita.
   ----------------------------------------------------------------------- */
void InicioMuestra(){
	gettimeofday(&inicio, (void *)0);
}


/* -----------------------------------------------------------------------
   FinMuestra: registra el instante final, calcula e imprime el tiempo.
   La diferencia se calcula en microsegundos:
     - fin.tv_usec -= inicio.tv_usec  → diferencia de microsegundos
     - fin.tv_sec  -= inicio.tv_sec   → diferencia de segundos
   Conversión final: segundos×1,000,000 + microsegundos = total en µs.
   Este valor es el que lanzador.pl captura con ">>" y acumula en el .dat
   para calcular estadísticas de rendimiento (promedio, desviación estándar).
   ----------------------------------------------------------------------- */
void FinMuestra(){
	gettimeofday(&fin, (void *)0);
	fin.tv_usec -= inicio.tv_usec;   /* Resta microsegundos */
	fin.tv_sec  -= inicio.tv_sec;    /* Resta segundos */
	double tiempo = (double) (fin.tv_sec*1000000 + fin.tv_usec);  /* Convierte todo a µs */
	printf("%9.0f \n", tiempo);      /* Imprime con 9 dígitos, 0 decimales */
}


/* -----------------------------------------------------------------------
   iniMatrix: llena dos matrices con valores aleatorios double.
   - m1: primera matriz (rango [1.0, 5.0) )
   - m2: segunda matriz (rango [2.0, 9.0) )
   - D: dimensión, se generan D*D valores para cada matriz.

   srand(time(NULL)) inicializa la semilla con el tiempo actual para que
   cada ejecución produzca valores distintos.

   Fórmula: rand()/RAND_MAX produce un valor en [0.0, 1.0).
   Multiplicado por (max - min) y sumado min da el rango deseado.
   Ej. para m1: rand()/RAND_MAX * (5.0-1.0) ∈ [0.0, 4.0), pero falta sumar 1.0
   → en el código el min implícito es 1.0 porque se suma 1.0 literalmente como
   parte del rango: (5.0-1.0) da el ancho pero el offset base ya está implícito
   en que el resultado mínimo real sería 0.0... 
   NOTA: hay un bug sutil: debería ser rand()/RAND_MAX*(5.0-1.0)+1.0 para [1,5).
   Tal como está, m1 ∈ [0.0, 4.0) y m2 ∈ [0.0, 7.0). Sin embargo, para el
   propósito del benchmarking (medir tiempo, no validar resultados numéricos)
   esto no afecta las conclusiones del experimento.
   ----------------------------------------------------------------------- */
void iniMatrix(double *m1, double *m2, int D){
	srand(time(NULL));             /* Inicializa la semilla aleatoria con el tiempo actual */
	for(int i=0; i<D*D; i++, m1++, m2++){
		*m1 = (double)rand()/RAND_MAX*(5.0-1.0);   /* Valor aleatorio para matriz A */
		*m2 = (double)rand()/RAND_MAX*(9.0-2.0);   /* Valor aleatorio para matriz B */
	}
}

/* Cierre de la guarda de inclusión (ver nota al inicio del archivo) */
#endif
