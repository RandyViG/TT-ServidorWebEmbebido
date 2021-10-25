/* 
 * File:   SHT3xDis.h
 * Author: paolaraya
 *
 * Created on 22 de octubre de 2021, 19:14
 */

#ifndef SHT3XDIS_H
#define	SHT3XDIS_H

#ifdef	__cplusplus
extern "C" {
#endif
   

void iniciar_modulo_I2C( void );
void reiniciar_SHT(void);
unsigned char configurar_sensor(void);
unsigned char realizar_lectura(void);

#ifdef	__cplusplus
}
#endif

#endif	/* SHT3XDIS_H */

