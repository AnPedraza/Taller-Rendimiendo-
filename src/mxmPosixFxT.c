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
#include <pthread.h>   /* Biblioteca POSIX para hilos */
#include <unistd.h>    /* Funciones POSIX generales */
#include <stdlib.h>    /* malloc(), free(), calloc(), atoi(), exit() */
#include <time.h>      /* time() */
#include <sys/time.h>  /* gettimeofday() */
#include "moduloMM.h"  /* Módulo compartido: iniMatrix, impMatrix, matrixTRP,
                          InicioMuestra, FinMuestra */

/* -----------------------------------------------------------------------
   Variables globales compartidas entre todos los hilos.
   Los hilos de Pthreads comparten el espacio de direcciones del proceso,
   por eso pueden acceder a estas variables directamente sin parámetros.

   matrixT es la novedad frente a mxmPosixFxC: almacena la transpuesta de B.
   Todos los hilos leen matrixT (solo lectura) → no hay condición de carrera.
   ----------------------------------------------------------------------- */
pthread_mutex_t MM_mutex;
double *matrixA, *matrixB, *matrixT, *matrixC;

/* -----------------------------------------------------------------------
   Struct para pasar parámetros a cada hilo.
   pthread_create acepta un único void*, así que se empaquetan
   los tres valores necesarios en esta estructura.
   ----------------------------------------------------------------------- */
struct parametros{
        int nH;    /* Número total de hilos */
        int idH;   /* Identificador único de este hilo (0..nH-1) */
        int N;     /* Dimensión de la matriz cuadrada N×N */
};

/* -----------------------------------------------------------------------
   mxmPosixFxT: función que ejecuta cada hilo.
   Diferencia clave respecto a mxmPosixFxC: usa matrixT (transpuesta de B)
   en lugar de matrixB. Esto hace que ambos punteros (pA y pB) avancen
   de 1 en 1, convirtiendo el acceso en completamente secuencial.

   Distribución de trabajo:
   - El total de N filas se divide en nH bloques iguales.
   - Este hilo procesa las filas [filaI, filaF).
   - Asume que N es divisible por nH (garantizado por los parámetros del lanzador).
   ----------------------------------------------------------------------- */
void *mxmPosixFxT(void *variables){
	struct parametros *data = (struct parametros *)variables;
	int idH = data->idH;   /* ID de este hilo */
	int nH  = data->nH;    /* Total de hilos */
	int D   = data->N;     /* Dimensión N */

	/* Calcula el rango de filas asignado a este hilo */
	int filaI = (D/nH) * idH;       /* Primera fila a calcular (inclusiva) */
	int filaF = (D/nH) * (idH+1);   /* Primera fila que NO calcula (exclusiva) */

	/* Bucle de multiplicación con transpuesta */
    for (int i = filaI; i < filaF; i++){
        for (int j = 0; j < D; j++){
			double *pA, *pB, Suma = 0.0;
			pA = matrixA + i*D;   /* Inicio de la fila i en A */
			pB = matrixT + j*D;   /* Inicio de la fila j en mT
                                     (equivale a la columna j de B original,
                                     pero ahora almacenada de forma contigua) */

			/* CLAVE DE LA OPTIMIZACIÓN:
			   En FxC, pB avanzaba de D en D → saltos grandes → cache misses.
			   Aquí, pB avanza de 1 en 1 igual que pA → ambos accesos son
			   secuenciales → el hardware puede hacer prefetch eficientemente.
			   El beneficio es mayor cuanto más grande sea la matriz (D=3200
			   debería mostrar la diferencia más pronunciada). */
            for (int k = 0; k < D; k++, pA++, pB++){
				Suma += *pA * *pB;
			}
			matrixC[i*D+j] = Suma;   /* Escribe en la posición [i][j] de C.
                                        No hay conflicto: cada hilo escribe
                                        en filas distintas de C. */
		}
	}

	/* Mutex redundante: no protege ningún recurso compartido real.
	   El lock/unlock inmediato no tiene efecto práctico en este contexto.
	   La sincronización correcta se hace con pthread_join en el main. */
	pthread_mutex_lock (&MM_mutex);
	pthread_mutex_unlock (&MM_mutex);
	pthread_exit(NULL);
}

/* -----------------------------------------------------------------------
   main: punto de entrada, gestiona memoria, transpuesta y ciclo de hilos.
   ----------------------------------------------------------------------- */
int main(int argc, char *argv[]){

	/* Valida que se pasaron los dos argumentos requeridos */
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);	
	}
    int N      = (int) atoi(argv[1]);   /* Tamaño de la matriz */
    int num_Th = (int) atoi(argv[2]);   /* Número de hilos */

    pthread_t p[num_Th];       /* Array de IDs de hilo (VLA, requiere C99 o superior) */
    pthread_attr_t atrMM;      /* Atributos de los hilos */

	/* Reserva de memoria para las 4 matrices:
	   A, B (entrada), C (resultado) y T (transpuesta de B).
	   calloc inicializa a 0; el tamaño total es N*N*sizeof(double) por matriz.
	   Para N=3200: 3200*3200*8 = ~81 MB por matriz → ~324 MB en total. */
	matrixA = (double *)calloc(N*N, sizeof(double));
	matrixB = (double *)calloc(N*N, sizeof(double));
	matrixC = (double *)calloc(N*N, sizeof(double));
	matrixT = (double *)calloc(N*N, sizeof(double));   /* Extra respecto a FxC */

	/* Inicializa A y B con valores aleatorios y las muestra si N < 13 */
	iniMatrix(matrixA, matrixB, N);
	impMatrix(matrixA, N);
	impMatrix(matrixB, N);

	/* INICIO DE MEDICIÓN: el tiempo comienza aquí, justo antes de transponer.
	   La transposición SE INCLUYE en el tiempo medido porque es un costo
	   real de esta estrategia que no existe en FxC. Esto es correcto
	   desde el punto de vista del benchmarking comparativo. */
	InicioMuestra();

	/* Calcula la transpuesta de B y la guarda en matrixT.
	   Esta operación es O(N²) y se hace una sola vez, antes de los hilos.
	   El costo de transposición se amortiza frente a la ganancia en caché
	   durante la multiplicación, especialmente para N grande. */
	matrixTRP(N, matrixB, matrixT);

	/* Inicializa el mutex y configura atributos de hilo como joinable */
	pthread_mutex_init(&MM_mutex, NULL);
	pthread_attr_init(&atrMM);
	pthread_attr_setdetachstate(&atrMM, PTHREAD_CREATE_JOINABLE);

	/* Creación de los hilos: cada uno recibe su propia copia de parámetros */
    for (int j=0; j<num_Th; j++){
		/* malloc individual para evitar que todos los hilos lean la misma struct.
		   NOTA: no se libera este bloque (fuga de memoria menor).
		   Para un benchmark de corta duración, el SO lo recupera al terminar. */
		struct parametros *datos = (struct parametros *) malloc(sizeof(struct parametros)); 
		datos->idH = j;
		datos->nH  = num_Th;
		datos->N   = N;

		/* Lanza el hilo: ejecutará mxmPosixFxT con los parámetros dados */
        pthread_create(&p[j], &atrMM, mxmPosixFxT, (void *)datos);
	}

	/* Barrera de sincronización: el main espera a que cada hilo termine.
	   Después de este bucle, matrixC está completamente calculada. */
    for (int j=0; j<num_Th; j++)
        pthread_join(p[j], NULL);

	/* FIN DE MEDICIÓN: imprime el tiempo en µs → capturado por lanzador.pl */
	FinMuestra();
	
	/* Muestra el resultado si N < 13 */
	impMatrix(matrixC, N);

	/* Libera toda la memoria dinámica, incluyendo matrixT que no existe en FxC */
	free(matrixA);
	free(matrixB);
	free(matrixC);
	free(matrixT);   /* Exclusivo de esta versión con transpuesta */

	/* Limpieza de recursos de Pthreads */
	pthread_attr_destroy(&atrMM);
	pthread_mutex_destroy(&MM_mutex);

	/* pthread_exit en main: redundante después de los pthread_join,
	   pero garantiza que el proceso no termine antes que algún hilo rezagado.
	   Lo convencional sería "return 0;". */
	pthread_exit(NULL);

	return 0;
}
