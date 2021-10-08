/********************************************************************************************
 * @brief: Este programa hace la lectura de la interfaz de UART para obtener los valores
 *         enviados por el microcontrolador. Esta diseñado para funcionar en Linux, y 
 *         utiliza las llamadas a sistema como open y read para poder realizar la lectura.
 *         El microcontrolador se registra en el sistema en el directorio /dev y tiene
 *         como identificador el nombre: ttyUSBx
********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define EVER 1

int configurar_serial ( char *, speed_t );

int main( void ){
	int fd_serie;
	unsigned char dato[2];
	unsigned short int temp, hum;
	float t,h;

	fd_serie = configurar_serial( "/dev/ttyUSB0", B9600 );
	printf("serial abierto con descriptor: %d\n", fd_serie);

	for( ; EVER ; ){
		read( fd_serie, &dato[0], 1 );
		read( fd_serie, &dato[1], 1 );
		temp = dato[0];
		temp = temp<<8 | dato[1];

		read( fd_serie, &dato[0], 1 );
		read( fd_serie, &dato[1], 1 );
		hum = dato[0];
		hum = hum<<8 | dato[1];

		t = 175.0 * ( ( (float)temp)/65535.0 ) - 45.0;
		h = 100.0 * ( ( (float)hum)/65535.0 );
		printf("Temp: %f, Hum:%f\n", t, h);
	}
	close( fd_serie );

	return 0;
}

/********************************************************************************************
 *  @brief: Esta funcion configura la interfaz serie
 *  @param: dispositivo_serial, Nombre el dispositivo serial a usar (ej. ttyUSB0)
 *  @param: baudios, Velocidad de comunicacion. Se usa la constante Bxxxx, donde xxxx  es la
 *          velocidad. Las constantes de velocidad estan definidas en la cabecera termios.h
 *  @return: fd, Descriptor del serial
 *******************************************************************************************/
int configurar_serial( char *dispositivo_serial, speed_t baudios ){
	struct termios newtermios;
  	int fd;

/********************************************************************************************
 *  Se abre un descriptor de archivo para manejar la interfaz serie
 *   Banderas:
 *    O_RDWR - Se abre el descriptor para lectura y escritura
 *    O_NOCTTY - El dispositivo terminal no se convertira en el terminal del proceso
 *    ~O_NONBLOCK - Se hace bloqueante la lectura de datos
 *******************************************************************************************/
  	fd = open( dispositivo_serial, (O_RDWR | O_NOCTTY) & ~O_NONBLOCK );
	if( fd == -1 ){
		printf("Error al abrir el dispositivo tty \n");
		exit( EXIT_FAILURE );
  	}

/********************************************************************************************
 * cflag - Proporciona los indicadores de modo de control
 *	 CBAUD	- Velocidad de transmision en baudios.
 * 	 CS8	- Especifica los bits por dato, en este caso 8
 * 	 CLOCAL - Ignora las lineas de control del modem: CTS y RTS
 * 	 CREAD  - Habilita el receptor del UART
 * iflag - proporciona los indicadores de modo de entrada
 * 	 IGNPAR - Ingnora errores de paridad, es decir, comunicación sin paridad
 * oflag - Proporciona los indicadores de modo de salida
 * lflag - Proporciona indicadores de modo local
 * 	 TCIOFLUSH - Elimina datos recibidos pero no leidos, como los escritos pero no transmitidos
 * 	 ~ICANON - Establece modo no canonico, en este modo la entrada esta disponible inmediatamente
 * cc[]  - Arreglo que define caracteres especiales de control
 *	  VMIN > 0, VTIME = 0 - Bloquea la lectura hasta que el numero de bytes (1) esta disponible
 *******************************************************************************************/
	newtermios.c_cflag 	= CBAUD | CS8 | CLOCAL | CREAD;
  	newtermios.c_iflag 	= IGNPAR;
  	newtermios.c_oflag	= 0;
  	newtermios.c_lflag 	= TCIOFLUSH | ~ICANON;
  	newtermios.c_cc[VMIN]	= 1;
  	newtermios.c_cc[VTIME]	= 0;

	// Configura la velocidad de salida del UART
  	if( cfsetospeed( &newtermios, baudios ) == -1 ){
		printf("Error al establecer velocidad de salida \n");
		exit( EXIT_FAILURE );
  	}
	// Configura la velocidad de entrada del UART
	if( cfsetispeed( &newtermios, baudios ) == -1 ){
		printf("Error al establecer velocidad de entrada \n" );
		exit( EXIT_FAILURE );
	}
	// Limpia el buffer de entrada
	if( tcflush( fd, TCIFLUSH ) == -1 ){
		printf("Error al limpiar el buffer de entrada \n" );
		exit( EXIT_FAILURE );
	}
	// Limpia el buffer de salida
	if( tcflush( fd, TCOFLUSH ) == -1 ){
		printf("Error al limpiar el buffer de salida \n" );
		exit( EXIT_FAILURE );
	}
/********************************************************************************************
 * Se establece los parametros de terminal asociados con el descriptor de archivo fd 
 * utilizando la estructura termios
 * TCSANOW - Cambia los valores inmediatamente
 /******************************************************************************************/
	if( tcsetattr( fd, TCSANOW, &newtermios ) == -1 ){
		printf("Error al establecer los parametros de la terminal \n" );
		exit( EXIT_FAILURE );
	}
	
	return fd;
}
