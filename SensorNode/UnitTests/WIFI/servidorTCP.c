#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>

#define PUERTO 6000	//Número de puerto asignado al servidor
#define COLA_CLIENTES 5 //Tamaño de la cola de espera para clientes
#define TAM_BUFFER 6


int main(int argc, char **argv){
	int sockfd, cliente_sockfd;
	struct sockaddr_in direccion_servidor;
	unsigned char trama[TAM_BUFFER];
	unsigned short int idNodo, dato;
	unsigned char idSensor;
	float voCas=0, voSensor=0, lel=0, t=0, h=0;

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


		idNodo = trama[0];
		idNodo = idNodo << 8 | trama[1];

		idSensor = trama[2];

		dato = trama[3];
		dato = dato << 8 | trama[4]; 
		
		printf("Nodo: %u\n", idNodo);

		switch(idSensor){
			case 0:
				t = 175.0 * ( ((float)dato)/65535.0 ) - 45.0;
				printf("Sensor: Tempearura\n Temp: %f\n", t);
			break;
			case 1:
				h = 100.0 * ( ((float)dato)/65535.0 );
				printf("Sensor: Humedad\n Hum: %f\n", h);
			break;
			case 2:
				voCas = (float)dato * ( 3.3 / 4096.0 );
				voSensor = voCas / 8.0;
				lel = voSensor / ( ( (2/45)*h + (1/20)*t + 25.33 ) * 0.001 );
				printf("Digital: %u - VCas: %f - Vs: %f\n", dato, voCas, voSensor);
				printf("Sensor: Gas LP\n LEL: %f\n", lel);
			break;
			default:
				printf("Sensor not found\n");
			break;
		}

		close(cliente_sockfd);
	}
	
	close( sockfd );

	return 0;
}

