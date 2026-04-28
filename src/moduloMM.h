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

#ifndef __MODULOMM_H__
#define __MODULOMM_H__

///* Llenado de matrices A y B con valores aleatorios para las pruebas 
void iniMatrix(double *m1, double *m2, int D);

/* Registro del instante inicial para el cálculo del tiempo de ejecución */
void InicioMuestra();

/* Registro del instante final para obtener el tiempo total de la corrida */
void FinMuestra();

/* Visualización de la matriz en consola para validar resultados en matrices 6x6 */
void impMatrix(double *matrix, int D);

/* Creación de la matriz transpuesta para optimizar la localidad espacial */
void matrixTRP(int N, double *mB, double *mT);

/* Multiplicación paralela usando procesos fork con acceso por columnas */
void mxmForkFxC(double *mA, double *mB, double *mC, int D, int filaI, int filaF);


/* Multiplicación paralela usando procesos fork con acceso por filas */
void mxmForkFxT(double *mA, double *mT, double *mC, int D, int filaI, int filaF);

#endif
