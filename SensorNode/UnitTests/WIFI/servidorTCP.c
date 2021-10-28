#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define PUERTO 6000	//Número de puerto asignado al servidor
#define COLA_CLIENTES 5 //Tamaño de la cola de espera para clientes
#define TAM_BUFFER 5


int main(int argc, char **argv){
	int sockfd, cliente_sockfd;
	struct sockaddr_in direccion_servidor;
	unsigned char trama[TAM_BUFFER];
	unsigned short int idNodo, dato;
	unsigned char idSensor;

	memset( &direccion_servidor, 0, sizeof(direccion_servidor) );
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_port = htons(PUERTO);
	direccion_servidor.sin_addr.s_addr = INADDR_ANY;

	printf("Creando Socket ....\n");
	if( ( sockfd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ){
		perror("Ocurrio un problema en la creacion del socket\n");
		exit(1);
	}

	printf("Configurando socket ...\n");
	if( bind( sockfd, (struct sockaddr *)&direccion_servidor, sizeof(direccion_servidor) ) < 0 ){
		perror ("Ocurrio un problema al configurar el socket\n");
		exit(1);
	}

	printf("Estableciendo la aceptacion de clientes...\n");
	if( listen(sockfd, COLA_CLIENTES) < 0 ){
		perror("Ocurrio un problema al crear la cola de aceptar peticiones de los clientes\n");
		exit(1);
	}
	for( ; 1 ; ){
		printf("En espera de peticiones de conexión ...\n");
		if( ( cliente_sockfd = accept(sockfd, NULL, NULL) ) < 0 ){
			perror("Ocurrio algun problema al atender a un cliente");
			exit(1);
		}

		printf("Se aceptó un cliente, atendiendolo \n");
		if( read(cliente_sockfd, trama, TAM_BUFFER) < 0 ){
			perror ("Ocurrio algun problema al recibir datos del cliente");
			exit(1);
		}
		
		memcpy( &idNodo, trama, 2 );
		memcpy( &idSensor, trama+2, 1 );
		memcpy( &dato, trama+3, 2 );
	}
	

	close( cliente_sockfd );
	close( sockfd );

	return 0;
}

