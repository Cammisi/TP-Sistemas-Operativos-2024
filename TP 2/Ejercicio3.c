#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define ARRAY_SIZE 5

void mostrar_datos_grupo();

int main() {
	int numbers[ARRAY_SIZE] = {1, 2, 3, 4, 5};  // Conjunto de datos
	int sum_pipe[2], product_pipe[2];
	
	mostrar_datos_grupo();
		
	// Crear pipes
	if (pipe(sum_pipe) == -1 || pipe(product_pipe) == -1) {
		perror("Error al crear los pipes");
		return 1;
	}
	
	pid_t pid1 = fork();  // Primer fork para el proceso hijo 1
	if (pid1 == -1) {
		perror("Error al crear el primer hijo");
		return 1;
	}
	
	if (pid1 == 0) {
		// Proceso hijo 1: Calcula la suma de los números
		close(sum_pipe[0]);  // Cerrar el extremo de lectura del pipe de suma
		
		int sum = 0;	
		int i;
		for ( i = 0; i < ARRAY_SIZE; i++) {
			sum += numbers[i];
		}
		
		if (write(sum_pipe[1], &sum, sizeof(sum)) == -1) {// el write envia la suma al padre
			perror("Error al escribir en el pipe de suma");
			close(sum_pipe[1]);
			exit(1);
		}  
		close(sum_pipe[1]);  // Cerrar el extremo de escritura del pipe de suma
		
		exit(0);
	}
	
	pid_t pid2 = fork();  // Segundo fork para el proceso hijo 2
	if (pid2 == -1) {
		perror("Error al crear el segundo hijo");
		return 1;
	}
	
	if (pid2 == 0) {
		// Proceso hijo 2: Calcula el producto de los números
		close(product_pipe[0]);  // Cerrar el extremo de lectura del pipe de producto
		
		int product = 1;
		int i;
		for (i = 0; i < ARRAY_SIZE; i++) {
			product *= numbers[i];
		}
		
		if (write(product_pipe[1], &product, sizeof(product)) == -1) {//El write envia el producto al padre
			perror("Error al escribir en el pipe de producto");
			close(product_pipe[1]);
			exit(1);
		}		
		close(product_pipe[1]);  // Cerrar el extremo de escritura del pipe de producto
		
		exit(0);
	}
	
	// Proceso padre
	close(sum_pipe[1]);      // Cerrar el extremo de escritura del pipe de suma
	close(product_pipe[1]);  // Cerrar el extremo de escritura del pipe de producto
	
	int sum, product;
	if (read(sum_pipe[0], &sum, sizeof(sum)) == -1) {//lee la suma del hijo 1
		perror("Error al leer del pipe de suma");
		close(sum_pipe[0]);
		close(product_pipe[0]);
		return 1;
	}
	
	if (read(product_pipe[0], &product, sizeof(product)) == -1) {//lee el producto del hijo 2
		perror("Error al leer del pipe de producto");
		close(sum_pipe[0]);
		close(product_pipe[0]);
		return 1;
	}
	
	close(sum_pipe[0]);     // Cerrar el extremo de lectura del pipe de suma
	close(product_pipe[0]); // Cerrar el extremo de lectura del pipe de producto
	
	// Escribir los resultados en un archivo
	FILE *file = fopen("resultados.txt", "w");
	if (file == NULL) {
		perror("Error al abrir el archivo");
		return 1;
	}
	
	fprintf(file, "Suma: %d\n", sum);
	fprintf(file, "Producto: %d\n", product);
	fclose(file);
	
	// Leer el archivo y mostrar el contenido en pantalla
	file = fopen("resultados.txt", "r");
	if (file == NULL) {
		perror("Error al leer el archivo");
		return 1;
	}
	
	char line[256];
	while (fgets(line, sizeof(line), file)) {
		printf("%s", line);
	}
	fclose(file);
	
	// Esperar a que terminen ambos hijos
	if (wait(NULL) == -1) {
		perror("Error al esperar al primer hijo");
		return 1;
	}
	
	if (wait(NULL) == -1) {
		perror("Error al esperar al segundo hijo");
		return 1;
	}
	
	return 0;
}


void mostrar_datos_grupo() {
	printf("Grupo: 14\n");
	printf("Integrantes:\n");
	printf("Cammisi Jose - jose_cammisi@hotmail.com.ar\n");
	printf("Melgratti Nicolas -nicomelgratti@gmail.com\n");
	printf("Tessini  Valentin  -valentin-tessini@hotmail.com\n");
}
