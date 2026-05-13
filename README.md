# Taller de Evaluacion de Rendimiento

## Pontificia Universidad Javeriana
### Facultad de Ingenieria - Departamento de Ingenieria de Sistemas
**Materia:** Sistemas Operativos - 2026-10
**Profesor:** John Corredor
**Autor:** David Pedraza

---

## Descripcion
Este proyecto implementa un entorno de evaluacion de rendimiento para comparar el desempeno de cuatro algoritmos de multiplicacion de matrices en sistemas Linux. El objetivo es analizar la eficiencia de versiones paralelas mediante el uso de procesos (Fork) e hilos (POSIX Pthreads), variando el tamano de la matriz y el numero de hilos/procesos.

Para garantizar validez estadistica, los resultados se recolectan automaticamente mediante un script en Perl, aplicando la Ley de los Grandes Numeros con 30 repeticiones por cada configuracion.

---

## Algoritmos Implementados

| Ejecutable | Mecanismo | Algoritmo | Categoria |
| :--- | :--- | :--- | :--- |
| mxmPosixFxC | Pthreads | Clasico | Cache-unfriendly |
| mxmPosixFxT | Pthreads | Transpuesta | Cache-friendly |
| mxmForkFxC | Fork | Clasico | Cache-unfriendly |
| mxmForkFxT | Fork | Transpuesta | Cache-friendly |

---

## Estructura del Proyecto

```text
evalRend2610/
├── src/                 # Codigo fuente (.c y .h)
├── bin/                 # Ejecutables compilados
├── Soluciones/          # Archivos de resultados (.dat)
├── lanzador.pl          # Script de automatizacion
└── Makefile             # Archivo de compilacion
```
CompilacionDesde la raiz del proyecto:
```
make All
```
Verificar que se crearon los 4 ejecutables en la carpeta bin:
```
ls bin/
```
##Ejecucion de Experimentos
1. Verificacion rapidaProbar un ejecutable con una matriz de 6x6 y 2 hilos/procesos:\
```
./bin/mxmPosixFxC 6 2
```
3. Lanzar bateria de pruebasEl script lanzador.pl realiza las repeticiones de forma automatica segun la configuracion definida.
   ```
# Limpiar datos previos
```
rm -f Soluciones/*.dat
```

## Ejecutar en segundo plano
```
nohup ./lanzador.pl &
```
##3. Monitoreo
Para verificar el progreso de la generacion de archivos (Total esperado: 48):
```
watch -n 10 'ls Soluciones/ | wc -l'
```
##Analisis de Resultados
Los resultados se almacenan en la carpeta Soluciones/. Cada archivo contiene los tiempos de ejecucion en microsegundos
#.Calculo de promedioBash
```
awk '{sum+=$1} END {printf "Promedio: %.2f us\n", sum/NR}' Soluciones/NOMBRE_ARCHIVO.dat
```
## Plataformas utilizadas

| Plataforma | CPU | Nucleos | RAM | Entorno |
|---|---|---|---|---|
| Laptop AMD (Victus) | AMD Ryzen 5 5600H | 6 | 16 GB | WSL2 Ubuntu |
| Laptop Intel | Intel Core i5 11va Gen | 4 | 8 GB | WSL2 Ubuntu |
| Servidor Virtual | Intel Xeon Gold | 4 vCPU | 12 GB | VMware Linux |
