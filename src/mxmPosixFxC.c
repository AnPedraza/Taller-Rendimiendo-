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
#include <pthread.h>   /* Biblioteca POSIX para manejo de hilos */
#include <unistd.h>    /* getpid(), sleep(), etc. */
#include <stdlib.h>    /* malloc(), free(), atoi(), exit() */
#include <time.h>      /* time() para srand() */
#include <sys/time.h>  /* gettimeofday() para medición de tiempo */
#include "moduloMM.h"  /* Funciones compartidas: iniMatrix, impMatrix, InicioMuestra, FinMuestra */

/* -----------------------------------------------------------------------
   Variables globales accesibles por TODOS los hilos.
   Al ser globales, cada hilo puede leer matrixA, matrixB y escribir en matrixC
   sin necesidad de pasar punteros por argumento (ya los tiene disponibles).
   Esto es posible porque los hilos comparten el espacio de memoria del proceso padre.
   ----------------------------------------------------------------------- */
pthread_mutex_t MM_mutex;          /* Mutex de sincronización (ver nota en la función hilo) */
double *matrixA, *matrixB, *matrixC;  /* Punteros a las matrices en memoria dinámica */

/* -----------------------------------------------------------------------
   Estructura para empaquetar los parámetros de cada hilo.
   pthread_create solo acepta un único void* como argumento, así que
   se empaquetan los tres datos necesarios en una struct.
   - nH:  número total de hilos activos (para calcular el reparto de filas)
   - idH: identificador del hilo (0, 1, 2, ..., nH-1)
   - N:   dimensión de las matrices cuadradas N×N
   ----------------------------------------------------------------------- */
struct parametros{
	int nH;
	int idH;
	int N;
};

/* -----------------------------------------------------------------------
   mxmPosixFxC: función que ejecuta cada hilo.
   Recibe un void* que apunta a una struct parametros con su configuración.
   Cada hilo calcula un subconjunto de filas de la matriz resultado C.

   Estrategia de división del trabajo:
   - El rango de filas se divide equitativamente entre los nH hilos.
   - Hilo 0 procesa filas [0, D/nH)
   - Hilo 1 procesa filas [D/nH, 2*D/nH)
   - Hilo k procesa filas [k*D/nH, (k+1)*D/nH)

   IMPORTANTE: este reparto asume que N es divisible exactamente por nH.
   Los tamaños usados (200, 800, 1600, 3200) y los hilos (1, 2, 4) siempre
   cumplen esta condición, por lo que no se pierden filas sin calcular.

   Algoritmo interno: multiplicación clásica fila × columna.
   pA recorre la fila i de A de forma secuencial (cache-friendly).
   pB recorre la columna j de B con saltos de D posiciones (cache-unfriendly).
   ----------------------------------------------------------------------- */
void *mxmPosixFxC(void *variables){
	struct parametros *data = (struct parametros *)variables;
	/* Se extraen los valores de la struct para mayor legibilidad */
	int idH = data->idH;   /* ID de este hilo */
	int nH  = data->nH;    /* Total de hilos */
	int D   = data->N;     /* Dimensión de la matriz */

	/* Calcula el rango de filas que le corresponde procesar a este hilo */
	int filaI = (D/nH) * idH;       /* Fila inicial (inclusiva) */
	int filaF = (D/nH) * (idH+1);   /* Fila final (exclusiva) */

	/* Bucle principal: cada hilo solo itera sobre su rango de filas */
    for (int i = filaI; i < filaF; i++){
        for (int j = 0; j < D; j++){
			double *pA, *pB, Suma = 0.0;
			pA = matrixA + i*D;  /* Inicio de la fila i en A (avanza de 1 en 1) */
			pB = matrixB + j;    /* Inicio de la columna j en B (avanza de D en D) */
            for (int k = 0; k < D; k++, pA++, pB+=D){
				/* pB salta D posiciones → acceso no secuencial → muchos cache misses
				   Con D=3200 cada salto es 3200×8 bytes = 25,600 bytes,
				   muy superior al tamaño de una línea de caché (64 bytes típicamente) */
				Suma += *pA * *pB;
			}
			matrixC[i*D+j] = Suma;  /* Escribe el resultado; no hay conflicto entre hilos
                                       porque cada hilo escribe en filas distintas de C */
		}
	}

	/* NOTA sobre el mutex:
	   El lock/unlock aquí no protege ninguna sección crítica real.
	   Cada hilo ya escribe en rangos disjuntos de matrixC, por lo que
	   no existe condición de carrera. El mutex es redundante.
	   La sincronización real se logra con pthread_join en el main. */
	pthread_mutex_lock (&MM_mutex);
	pthread_mutex_unlock (&MM_mutex);
	pthread_exit(NULL);   /* Termina el hilo limpiamente */
}

/* -----------------------------------------------------------------------
   main: función principal, crea y coordina los hilos.
   ----------------------------------------------------------------------- */
int main(int argc, char *argv[]){
	/* Validación de argumentos: se necesitan exactamente 2 (tamaño y num hilos) */
	if (argc < 3){
		printf("Ingreso de argumentos \n $./ejecutable tamMatriz numHilos\n");
		exit(0);	
	}
    int N      = (int) atoi(argv[1]);   /* Dimensión de la matriz N×N */
    int num_Th = (int) atoi(argv[2]);   /* Número de hilos a crear */

    pthread_t p[num_Th];       /* Array de identificadores de hilo (VLA en C99) */
    pthread_attr_t atrMM;      /* Atributos de configuración para los hilos */

	/* Reserva memoria dinámica para las tres matrices (N×N doubles cada una).
	   calloc inicializa todo a 0, garantizando que matrixC comience limpia. */
	matrixA = (double *)calloc(N*N, sizeof(double));
	matrixB = (double *)calloc(N*N, sizeof(double));
	matrixC = (double *)calloc(N*N, sizeof(double));

	/* Llena A y B con valores aleatorios y las imprime si N < 13 */
	iniMatrix(matrixA, matrixB, N);
	impMatrix(matrixA, N);
	impMatrix(matrixB, N);

	/* INICIO DE LA MEDICIÓN DE TIEMPO.
	   Todo lo que sucede antes (iniMatrix, impMatrix, calloc) NO se mide,
	   garantizando que el tiempo capturado corresponda solo a la multiplicación. */
	InicioMuestra();

	/* Inicialización del mutex con atributos por defecto (NULL) */
	pthread_mutex_init(&MM_mutex, NULL);

	/* Inicialización y configuración de atributos de hilo:
	   PTHREAD_CREATE_JOINABLE permite usar pthread_join para esperar al hilo. */
	pthread_attr_init(&atrMM);
	pthread_attr_setdetachstate(&atrMM, PTHREAD_CREATE_JOINABLE);

	/* Creación de los hilos: cada uno recibe su propia struct parametros */
    for (int j=0; j<num_Th; j++){
		/* Se hace malloc individual para cada hilo para evitar que todos
		   compartan la misma struct (lo que causaría condición de carrera
		   si se reutilizara una sola struct en el stack del main).
		   NOTA: falta free(datos) → fuga de memoria de num_Th × sizeof(struct parametros).
		   Para un benchmark de corta duración no es crítico, pero es un defecto. */
		struct parametros *datos = (struct parametros *) malloc(sizeof(struct parametros)); 
		datos->idH = j;        /* ID único de este hilo */
		datos->nH  = num_Th;   /* Total de hilos (para calcular el reparto) */
		datos->N   = N;        /* Dimensión de las matrices */

		/* pthread_create lanza el hilo:
		   - p[j]:          identificador resultante del hilo
		   - &atrMM:        atributos (joinable)
		   - mxmPosixFxC:   función que ejecutará el hilo
		   - (void*)datos:  argumento empaquetado para el hilo */
        pthread_create(&p[j], &atrMM, mxmPosixFxC, (void *)datos);
	}

	/* Espera a que cada hilo termine antes de continuar.
	   pthread_join bloquea el main hasta que el hilo p[j] llame pthread_exit.
	   Garantiza que matrixC esté completamente calculada antes de FinMuestra(). */
    for (int j=0; j<num_Th; j++)
        pthread_join(p[j], NULL);

	/* FIN DE LA MEDICIÓN: calcula e imprime el tiempo en microsegundos.
	   Este valor es el que lanzador.pl redirige con ">>" al archivo .dat */
	FinMuestra();
	
	/* Imprime la matriz resultado si N < 13 (para validación visual) */
	impMatrix(matrixC, N);

	/* Liberación de toda la memoria dinámica reservada */
	free(matrixA);
	free(matrixB);
	free(matrixC);

	/* Destruye los atributos y el mutex (libera recursos del sistema operativo) */
	pthread_attr_destroy(&atrMM);
	pthread_mutex_destroy(&MM_mutex);

	/* pthread_exit en el main hace que el proceso espere a que terminen
	   todos los hilos antes de salir. Aunque ya se hizo pthread_join para
	   cada hilo, este llamado es redundante pero no incorrecto.
	   Lo habitual sería simplemente "return 0;" */
	pthread_exit(NULL);

	return 0;
}
