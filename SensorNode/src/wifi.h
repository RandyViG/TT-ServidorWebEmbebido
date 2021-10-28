#ifndef WIFI_H
#define	WIFI_H

#ifdef	__cplusplus
extern "C" {
#endif

void iniciar_uart( void );
void iniciar_wifi( void );
void configurar_wifi( void );
void enviar_wifi(void);
void cerrar_conexion( void );

#ifdef	__cplusplus
}
#endif

#endif	/* WIFI_H */

