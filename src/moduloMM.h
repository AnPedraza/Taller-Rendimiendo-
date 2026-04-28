/*#######################################################################################
 #* Fecha: 
 #* Autor: J. Corredor, PhD
 #* Modulo: 
 #       -     
 #* Versión:
 #*	 	Concurrencia de Tareas: Paralelismo sobre Multiplicación de Matrices
 #* Descripción:
 #       - 
######################################################################################*/

#ifndef __MODULOMM_H__
#define __MODULOMM_H__

void iniMatrix(double *m1, double *m2, int D);

void InicioMuestra();

void FinMuestra();

void impMatrix(double *matrix, int D);

void matrixTRP(int N, double *mB, double *mT);

void mxmForkFxC(double *mA, double *mB, double *mC, int D, int filaI, int filaF);

void mxmForkFxT(double *mA, double *mT, double *mC, int D, int filaI, int filaF);

#endif
