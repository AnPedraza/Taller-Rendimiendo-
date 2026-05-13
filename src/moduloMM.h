/*#######################################################################################
 #		Pontificia Universidad Javeriana
 #* Fecha:27 de abril de 2026
 #* Autores: Oscar Pinilla, Johan Barreto, David Pedraza
 #* Modulo: 
 #       -     moduloMM.h
 #* Versión:
 #*	 	Concurrencia de Tareas: Paralelismo sobre Multiplicación de Matrices
 #* Descripción:
 #       - Este archivo define la interfaz para el manejo y multiplicación de matrices. 
 #       Incluye prototipos para la inicialización, impresión, transposición y las 
 #       variantes de multiplicación paralela utilizando procesos
######################################################################################*/

/* Guarda de inclusión: evita que este header sea procesado más de una vez
   si se incluye desde varios archivos .c en la misma unidad de compilación. */
#ifndef __MODULOMM_H__
#define __MODULOMM_H__

/* -------------------------------------------------------------------
   PROTOTIPOS DE FUNCIONES
   Todos están implementados en moduloMM.c
   ------------------------------------------------------------------- */

/* Llena las matrices m1 y m2 con valores aleatorios tipo double.
   m1 recibe valores en el rango [1.0, 5.0)
   m2 recibe valores en el rango [2.0, 9.0)
   D es la dimensión: se llenan D*D elementos en cada matriz. */
void iniMatrix(double *m1, double *m2, int D);

/* Registra el instante de inicio de la medición de tiempo.
   Internamente llama a gettimeofday() y guarda el resultado
   en la variable global 'inicio' de tipo struct timeval. */
void InicioMuestra();

/* Registra el instante final, calcula la diferencia respecto al inicio
   e imprime el tiempo transcurrido en microsegundos (sin salto de línea adicional).
   El valor impreso es el que captura lanzador.pl y guarda en los archivos .dat */
void FinMuestra();

/* Imprime la matriz en consola con formato legible.
   Solo imprime si D < 13, para no saturar la consola con matrices grandes.
   El parámetro D es la dimensión (la matriz tiene D*D elementos). */
void impMatrix(double *matrix, int D);

/* Genera la transpuesta de mB y la guarda en mT.
   La transposición convierte el acceso por columnas en acceso por filas,
   mejorando la localidad espacial en caché durante la multiplicación.
   N es la dimensión de la matriz cuadrada. */
void matrixTRP(int N, double *mB, double *mT);

/* Multiplicación clásica fila por columna usando punteros.
   mA: matriz A, mB: matriz B sin transponer, mC: resultado.
   D: dimensión de las matrices cuadradas.
   filaI..filaF: rango de filas de A que este proceso/hilo debe calcular.
   El acceso a mB avanza de D en D (saltos grandes) → cache-unfriendly. */
void mxmForkFxC(double *mA, double *mB, double *mC, int D, int filaI, int filaF);

/* Multiplicación optimizada usando la transpuesta de B.
   mA: matriz A, mT: matriz B ya transpuesta, mC: resultado.
   D: dimensión, filaI..filaF: rango de filas asignado a este proceso/hilo.
   Ambos punteros avanzan de 1 en 1 → acceso secuencial → cache-friendly. */
void mxmForkFxT(double *mA, double *mT, double *mC, int D, int filaI, int filaF);

/* Cierre de la guarda de inclusión */
#endif
