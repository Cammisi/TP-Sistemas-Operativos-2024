#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

// Declaración de prototipos
void mostrar_datos_grupo();
void obtener_nombre_proceso(pid_t pid);
void obtener_estado_proceso(pid_t pid);
void obtener_uso_memoria(pid_t pid);
void obtener_uso_cpu(pid_t pid);
void obtener_prioridad_proceso(pid_t pid);

// Función main
int main(int argc, char *argv[]) {//argc es la cantidad de parametros (nombre del programa y el pid) nos aseguramos que sea=2
	if (argc != 2) {
		fprintf(stderr, "Sintaxis incorrecta. La Sintaxis correcta es: %s PID.\n", argv[0]);
		return 1;
	}
	
	pid_t pid = atoi(argv[1]);//toma el valor del pid
	if (pid <= 0) {//el pid debe ser mayor a cero
		fprintf(stderr, "Error: PID inválido\n");
		return 1;
	}
	
	// Mostramos los datos del grupo
	mostrar_datos_grupo();
	
	printf("\nMonitoreando el proceso con PID: %d\n", pid);
	
	// Llamadas a funciones
	obtener_nombre_proceso(pid);
	obtener_estado_proceso(pid);
	obtener_uso_memoria(pid);
	obtener_uso_cpu(pid);
	obtener_prioridad_proceso(pid);
	
	return 0;
}

// Implementación de prototipos
void mostrar_datos_grupo() {
	printf("Grupo: 14\n");
	printf("Integrantes:\n");
	printf("Cammisi Jose - jose_cammisi@hotmail.com.ar\n");
	printf("Melgratti Nicolas -nicomelgratti@gmail.com\n");
	printf("Tessini  Valentin  -valentin-tessini@hotmail.com\n");
}

void obtener_nombre_proceso(pid_t pid) {
	char ruta[40], buffer[1024], nombre[256];//se declaran cadenas de caracteres, ruta es la ruta del archivo que tiene el nombre del proceso, y nombre es el nombre del proceso.
		// buffer para leer el contenido del archivo
	snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);//snprintf asegura que la ruta no sobrepase los límites del array ruta.
	
	int archivo = open(ruta, O_RDONLY);//abre el archivo en modo lectura
	if (archivo >= 0) {//verifica que el archivo se abrio correctamente
		ssize_t bytes_leidos = read(archivo, buffer, sizeof(buffer) - 1);//lee el contenido del archivo en buffer
		if (bytes_leidos > 0) {
			buffer[bytes_leidos] = '\0';// Aseguramos que el buffer termine en '\0' (fin de cadena)
			// Extraemos el segundo campo de la línea usando sscanf
			sscanf(buffer, "%*d (%[^)])", nombre);// Captura todo lo que está entre paréntesis en 'nombre'
			printf("Nombre del Proceso: %s\n", nombre);// Imprimimos el nombre del proceso
		} else {
			perror("Error al leer el nombre del proceso");//informa error
		}
		close(archivo);//cierra el archivo
	} else {
		perror("Error al abrir el archivo");//informa error
	}
}

void obtener_estado_proceso(pid_t pid) {
	char ruta[40], buffer[1024], estado;//se declara una cadena de caracteres ruta que es la ruta del archivo que tiene el nombre del proceso, y un char estado que almacena el estado actual del proceso.
	snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);//snprintf asegura que la ruta no sobrepase los límites del array ruta.
	//proc contiene diversa información del proceso, incluido su estado, en forma de una lista de valores. Por ejemplo, si el PID es 14, la ruta sería "/proc/14/stat".
	
	int archivo = open(ruta, O_RDONLY);//abre el archivo en modo lectura
	if (archivo >= 0) {//verifica que se abra el archivo
		ssize_t bytes_leidos = read(archivo, buffer, sizeof(buffer) - 1);//lee el archivo
		if (bytes_leidos > 0) {//si pudo leer el archivo
			buffer[bytes_leidos] = '\0';// Aseguramos que el buffer termine en '\0'
			sscanf(buffer, "%*d %*s %c", &estado);//lee el archivo y extrae el estado del proceso.%*d %*s ignoran los primeros dos campos, %c lee el tercero(que es el estado).
			printf("Estado del Proceso: ");
			switch (estado) {//se utiliza un switch para ver que valor tiene la variable estado y asi saber el estado del proceso
			case 'R': printf("Ejecutando\n"); break;
			case 'S': printf("Durmiendo\n"); break;
			case 'D': printf("Esperando\n"); break;
			case 'T': printf("Detenido\n"); break;
			case 'Z': printf("Zombie\n"); break;
			default: printf("Desconocido\n"); break;
			}
		} else {
			perror("Error al leer el estado del proceso");
		}
		close(archivo);//cierra el archivo
	} else {//no se pudo abrir el archivo
		perror("Error al abrir el archivo");
	}
}

void obtener_uso_memoria(pid_t pid) {
	char ruta[40], buffer[1024];//cadena de caracteres que contiene la ruta, buffer para leer el contenido del archivo.
	snprintf(ruta, sizeof(ruta), "/proc/%d/statm", pid);//proc/[PID]/statm lee el uso de memoria del proceso en términos de páginas de memoria.
	
	int archivo = open(ruta, O_RDONLY);//abre el archivo en modo lectura
	if (archivo >= 0) {//si el archivo existe
		ssize_t bytes_leidos = read(archivo, buffer, sizeof(buffer) - 1);//lee el contenido del archivo en buffer
		if (bytes_leidos > 0) {
			buffer[bytes_leidos] = '\0';// Aseguramos que el buffer termine en '\0'
			int memoria;//entero que contiene el valor de la memoria
			// Extraemos el primer valor del archivo (el número de páginas de memoria)
			sscanf(buffer, "%d", &memoria);
			printf("Uso de Memoria: %d KB\n", memoria * (sysconf(_SC_PAGESIZE) / 1024));//sysconf(_SC_PAGESIZE) para calcular el tamaño de página dinámicamente.
		} else {
			perror("Error al leer el uso de memoria");
		}
		close(archivo);//cierra el archivo
	} else {//no pudo abrir el archivo
		perror("Error al abrir el archivo");
	}
}

void obtener_uso_cpu(pid_t pid) {
	char ruta[40], buffer[1024];
	long utime, stime;
	snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid); // Ruta al archivo /proc/[PID]/stat
	
	// Abrimos el archivo /proc/[PID]/stat
	int archivo = open(ruta, O_RDONLY);
	if (archivo >= 0) {
		// Leemos el contenido del archivo en el buffer
		ssize_t bytes_leidos = read(archivo, buffer, sizeof(buffer) - 1);
		if (bytes_leidos > 0) {
			buffer[bytes_leidos] = '\0';// Aseguramos que el buffer termine en '\0'
			
			// Extraemos los valores de utime (14) y stime (15) desde el archivo
			sscanf(buffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %ld %ld", &utime, &stime);
			
			// Convertimos el uso de CPU a segundos
			long total_time = utime + stime;
			double segundos_cpu = (double)total_time / sysconf(_SC_CLK_TCK);
			
			// Imprimimos el uso de CPU
			printf("Uso de CPU: %.2f segundos\n", segundos_cpu);
		} else {
			perror("Error al leer el uso de CPU");
		}
		close(archivo);// Cerramos el archivo
	} else {
		perror("Error al abrir el archivo /proc/[PID]/stat");
	}
}

void obtener_prioridad_proceso(pid_t pid) {
	int prioridad = getpriority(PRIO_PROCESS, pid);//prioridad variable entera que inicializa con el resultado de getpriority
		//getpriority es una función que obtiene la prioridad de un proceso,PRIO_PROCESS indica que queremos obtener la prioridad de un proceso específico.
		//pid es el identificador del proceso del cual deseamos la prioridad.
	if (prioridad != -1 || errno == 0) {//comprueba si getpriority se ejecuto correctamente 
		//getpriority puede devolver -1 tanto en caso de error como cuando el valor de prioridad es realmente -1. 
		//Para distinguir entre ambos casos, se verifica que errno (es una variable global proporcionada por `<errno.h>` que almacena el código del último error ocurrido)
		//cuando la prioridad del proceso es realmente -1. En este caso, errno no cambia y permanece en 0
		printf("Prioridad del Proceso: %d\n", prioridad);
	} else {
		perror("Error al obtener la prioridad del proceso");
	}
}
