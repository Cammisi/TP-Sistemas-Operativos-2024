#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#define SIZE 9 // Espacios en la barcaza
void mostrar_datos_grupo();
typedef int semaforo;

// Estructura para manejar la memoria compartida
typedef struct {
	int espaciosLibres;
	int totalCamionesBloqueados;
	int camionesEnBarcaza;
} Barcaza;

// Semáforos
semaforo camiones_bloqueados, salida, mutex;

// Memoria compartida
int shmid;
Barcaza *estado;

// Funciones de inicialización
void inicializar_recursos();
void limpiar_recursos();
void inicializar_semaforo(semaforo sem, int valor);

// Funciones de sincronización
void down(semaforo sem);
void up(semaforo sem);

// Procesos
void barcaza();
void camion(int tamanio);

// Manejadores
void manejar_salida(int sig);

int main() {
	mostrar_datos_grupo();
	signal(SIGINT, manejar_salida); // Captura de Ctrl+C para limpiar recursos
	inicializar_recursos();
	
	if (fork() == 0) {
		barcaza(); // Proceso barcaza
		exit(0);
	}
	
	while (1) {
		if (fork() == 0) {
			srand(time(NULL));
			int tamanio = rand() % 2 + 1; // Camión normal (1) o con acoplado (2)
			camion(tamanio);
			exit(0);
		}
		sleep(2); // Crear camiones cada 2 segundos
	}
	
	return 0;
}

void inicializar_recursos() {
	key_t key = ftok(".", 'B'); // Crear una clave única basada en el archivo actual y un identificador
	if ((shmid = shmget(key, sizeof(Barcaza), IPC_CREAT | 0666)) == -1) {
		perror("Error creando memoria compartida");
		exit(1);
	}
	
	estado = (Barcaza *)shmat(shmid, NULL, 0);
	if (estado == (void *)-1) {
		perror("Error asociando memoria compartida");
		exit(1);
	}
	
	estado->espaciosLibres = SIZE;
	estado->totalCamionesBloqueados = 0;
	estado->camionesEnBarcaza = 0;
	
	// Crear semáforos con claves únicas generadas con ftok
	key_t key_camiones_bloqueados = ftok(".", 'C');
	key_t key_salida = ftok(".", 'S');
	key_t key_mutex = ftok(".", 'M');
	
	camiones_bloqueados = semget(key_camiones_bloqueados, 1, IPC_CREAT | 0666);
	salida = semget(key_salida, 1, IPC_CREAT | 0666);
	mutex = semget(key_mutex, 1, IPC_CREAT | 0666);
	
	if (camiones_bloqueados == -1 || salida == -1 || mutex == -1) {
		perror("Error creando semáforos");
		exit(1);
	}
	
	inicializar_semaforo(camiones_bloqueados, 0);
	inicializar_semaforo(salida, 0);
	inicializar_semaforo(mutex, 1);
}

void limpiar_recursos() {
	semctl(camiones_bloqueados, 0, IPC_RMID);
	semctl(salida, 0, IPC_RMID);
	semctl(mutex, 0, IPC_RMID);
	shmdt(estado);
	shmctl(shmid, IPC_RMID, NULL);
	printf("Recursos liberados correctamente.\n");
}

void inicializar_semaforo(semaforo sem, int valor) {
	union semun {
		int val;
	} arg;
	arg.val = valor;
	semctl(sem, 0, SETVAL, arg);
}

void down(semaforo sem) {
	struct sembuf op = {0, -1, 0};
	semop(sem, &op, 1);
}

void up(semaforo sem) {
	struct sembuf op = {0, 1, 0};
	semop(sem, &op, 1);
}

void barcaza() {
	while (1) {
		down(salida); // Espera a que la barcaza esté llena
		down(mutex);
		
		printf("Barcaza llena. Partiendo...\n");
		sleep(3); // Simula el viaje
		
		printf("Barcaza regresó vacía.\n");
		estado->espaciosLibres = SIZE; // Restablecer la capacidad
		estado->camionesEnBarcaza = 0;
		
		// Desbloquear a todos los camiones bloqueados
		if (estado->totalCamionesBloqueados > 0) {
			printf("Despertando a %d camiones bloqueados.\n", estado->totalCamionesBloqueados);
		}
		
		// Desbloquear a los camiones bloqueados
		int i;
		for (i = 0; i < estado->totalCamionesBloqueados; i++) {
			up(camiones_bloqueados); // Desbloquear camiones
		}
		
		// Resetear los camiones bloqueados
		estado->totalCamionesBloqueados = 0;
		
		up(mutex);
	}
}

void camion(int tamanio) {
	int bloqueado = 0; // Variable para saber si este camión fue bloqueado antes
	while (1) {
		down(mutex);
		
		if (estado->espaciosLibres >= tamanio) {
			if (bloqueado) {
				printf("Agregando camión bloqueado (%d espacios).\n", tamanio);
			} else {
				printf("Camión (%d espacios) cargado.\n", tamanio);
			}
			
			estado->espaciosLibres -= tamanio;
			estado->camionesEnBarcaza++;
			printf("Espacios libres: %d\n", estado->espaciosLibres);
			
			if (estado->espaciosLibres == 0) {
				// Notificar a la barcaza que está llena
				up(salida);
			}
			
			up(mutex);
			break;
		} else {
			if (!bloqueado) {
				printf("Camión (%d espacios) bloqueado.\n", tamanio);
				estado->totalCamionesBloqueados++;
				bloqueado = 1; // Marcar este camión como bloqueado
			}
			up(mutex);
			down(camiones_bloqueados); // Espera a ser desbloqueado
		}
	}
}

void manejar_salida(int sig) {
	limpiar_recursos();
	exit(0);
}
void mostrar_datos_grupo() {
	printf("Grupo: 14\n");
	printf("Integrantes:\n");
	printf("Cammisi Jose - jose_cammisi@hotmail.com.ar\n");
	printf("Melgratti Nicolas -nicomelgratti@gmail.com\n");
	printf("Tessini  Valentin  -valentin-tessini@hotmail.com\n");
}
