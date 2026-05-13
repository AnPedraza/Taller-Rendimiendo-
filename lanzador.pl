#!/usr/bin/perl
#**************************************************************
#         		Pontificia Universidad Javeriana
#     Autor: J. Corredor 
#	  Nombre Estudiante: David Pedraza
#     Fecha:11 de mayo de 2026
#     Materia: Sistemas Operativos
#     Tema: Taller de Evaluación de Rendimiento
#     Fichero: script automatización ejecución por lotes 
#****************************************************************/

# Obtiene la ruta absoluta del directorio actual de trabajo.
# El backtick `` ejecuta el comando 'pwd' en el shell y devuelve su salida.
# chomp() elimina el salto de línea '\n' que pwd agrega al final.
# $Path se usará para construir rutas absolutas a los ejecutables y resultados.
$Path = `pwd`;
chomp($Path);

# Lista de los 4 ejecutables a evaluar, que representan las 4 estrategias:
#   mxmPosixFxC: Pthreads + acceso por columnas (cache-unfriendly)
#   mxmPosixFxT: Pthreads + transpuesta        (cache-friendly)
#   mxmForkFxC:  Fork     + acceso por columnas (cache-unfriendly)
#   mxmForkFxT:  Fork     + transpuesta         (cache-friendly)
@Nombre_Ejecutable = ("mxmPosixFxC","mxmPosixFxT","mxmForkFxC","mxmForkFxT");

# Tamaños de matriz a evaluar: 200, 800, 1600 y 3200.
# Son potencias de 2 (o múltiplos), lo que garantiza divisibilidad exacta
# entre los 1, 2 y 4 hilos/procesos usados.
# El costo crece con O(N³) para la multiplicación, por lo que
# 3200 tarda aprox. (3200/200)^3 = 4096 veces más que 200.
@Size_Matriz = ("200","800","1600","3200");

# Número de hilos/procesos a probar: 1 (secuencial), 2 y 4 (paralelo).
# Permite analizar el speedup y la eficiencia del paralelismo:
# speedup = tiempo(1 hilo) / tiempo(N hilos)
@Num_Hilos = (1,2,4);

# Número de veces que se repite cada combinación.
# 30 repeticiones son suficientes para aplicar el Teorema Central del Límite
# y obtener un intervalo de confianza estadísticamente válido para la media.
$Repeticiones = 30;

# Nombre del directorio donde se guardan los archivos de resultados (.dat).
# Debe existir previamente: mkdir Soluciones
$Resultados = "Soluciones";

# Nombre del directorio donde están los ejecutables compilados.
# Debe existir y contener los 4 binarios compilados.
$Binarios = "bin";

# Triple bucle anidado: genera todas las combinaciones posibles.
# Total de combinaciones: 4 ejecutables × 4 tamaños × 3 hilos = 48 archivos .dat
# Cada archivo contiene 30 líneas con el tiempo en microsegundos de cada ejecución.
foreach $nombre (@Nombre_Ejecutable){
	foreach $size (@Size_Matriz){
		foreach $hilo (@Num_Hilos) {

			# Construye el nombre completo del archivo de resultados.
			# Ejemplo: /home/user/taller/Soluciones/mxmPosixFxC-800-Hilos-2.dat
			# La convención nombre-tamaño-Hilos-n facilita el análisis posterior.
			$file = "$Path/$Resultados/$nombre-".$size."-Hilos-".$hilo.".dat";
			printf("$file \n");   # Informa al usuario qué archivo se está generando

			# Repite la ejecución $Repeticiones (30) veces para la misma combinación.
			# Acumula los tiempos en el mismo archivo usando ">>" (append).
			for ($i=0; $i<$Repeticiones; $i++) {
				# Ejecuta el binario con los parámetros size e hilo.
				# La salida estándar del programa (el tiempo en µs que imprime FinMuestra)
				# se redirige con ">>" al archivo .dat (sin sobrescribir, siempre agrega).
				system("$Path/$Binarios/$nombre $size $hilo  >> $file");
				printf("$Path/$Binarios/$nombre $size $hilo \n");  # Log de progreso
			}

			# NOTA: close($file) aquí no hace nada útil.
			# $file es un string (la ruta del archivo), no un filehandle abierto en Perl.
			# El archivo se abre y cierra automáticamente en cada llamada a system()
			# a través de la redirección ">>" del shell. Es código muerto pero inofensivo.
			close($file);
		}
	}
}	
