#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define SIZE 1  // Tamaño del buffer de lectura en bytes
void mostrar_datos_grupo();

int main(int argc, char *argv[]) {
	mostrar_datos_grupo();//mostramos los datos del grupo
	
	int archivo_origen, archivo_destino;  // Descriptores de archivo para el origen y el destino
	ssize_t bytes_leidos = 0, bytes_escritos = 0, bytes_totales = 0; // Variables para la gestión de bytes
	struct stat info_archivo; // Estructura para almacenar información del archivo origen
	char buffer[SIZE];// Buffer de tamaño definido para leer los datos del archivo
	
	// Validar que se proporcionaron exactamente dos argumentos (nombre del archivo origen y destino)
	if (argc != 3) {
		fprintf(stderr, "Uso: %s <archivo_origen> <archivo_destino>\n", argv[0]);
		return 1;
	}
	
	// Abrir el archivo origen en modo de solo lectura
	archivo_origen = open(argv[1], O_RDONLY);
	if (archivo_origen == -1) {                        // Validar si hubo error al abrir
		perror("Error al abrir el archivo origen");
		return 1;
	}
	
	// Obtener información sobre el archivo origen
	stat(argv[1], &info_archivo);
	off_t size_archivo = info_archivo.st_size;// Obtener el tamaño total del archivo origen
	
	// Abrir/crear el archivo destino en modo de escritura, truncando su contenido si ya existe
	archivo_destino = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC);
	if (archivo_destino == -1) { // Validar si hubo error al crear o abrir
		perror("Error al crear el archivo de destino");
		close(archivo_origen);// Cerrar el archivo origen antes de salir
		return 1;
	}
	// Leer el archivo origen en bloques de tamaño 'SIZE' y escribir en el archivo destino
	bytes_leidos = read(archivo_origen, buffer, SIZE);// Leer los primeros bytes del archivo origen
	while (bytes_leidos > 0) {// Mientras haya bytes por leer
		
		bytes_escritos = write(archivo_destino, buffer, bytes_leidos);// Escribir los bytes leídos en el archivo destino
		
		if (bytes_escritos == -1) {// Validar si hubo error al escribir
			perror("Error al escribir en el archivo destino");
			close(archivo_origen);
			close(archivo_destino);
			return 1;
		}
		
		// Incrementar el contador de bytes totales escritos
		bytes_totales += bytes_escritos;
		float progreso = (float)bytes_totales / size_archivo * 100;// Calcular el porcentaje de progreso y mostrarlo en pantalla
		printf("\rProgreso: %.2f%%", progreso);// \r para sobrescribir en la misma línea
		fflush(stdout);// Asegurar que el texto se imprime en pantalla inmediatamente
		// Leer el siguiente bloque de datos
		bytes_leidos = read(archivo_origen, buffer, SIZE);
	}
	
	// Imprimir un mensaje final al completar la copia
	printf("\nFin\n");
	// Cerrar los archivos abiertos
	close(archivo_origen);
	close(archivo_destino);
	
	return 0;
}

void mostrar_datos_grupo() {
	printf("Grupo: 14\n");
	printf("Integrantes:\n");
	printf("Cammisi Jose - jose_cammisi@hotmail.com.ar\n");
	printf("Melgratti Nicolas -nicomelgratti@gmail.com\n");
	printf("Tessini  Valentin  -valentin-tessini@hotmail.com\n");
}
