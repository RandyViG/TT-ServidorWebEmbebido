#include"p30F4013.h"

#define NANCK 1
#define ACK 0

/*FUNCIONES PARA I2C*/
extern void iniciar_I2C( void );
extern void detener_I2C(void);
extern void enviar_dato_I2C( unsigned char dato );
extern unsigned char recibe_dato_I2C(void);
extern void ack_I2C(void);
extern void nack_I2C(void);

extern void iniciar_I2C(void);

extern void retardo_100ms();

extern short int humedad, temperatura;

/****************************************************************************/
/* @brief: ESTA FUNCI?N INICIALIZA I2C EN MODO FAST MODE 400KHz             */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
void iniciar_modulo_I2C( void ){
    I2CCON = 0X8000;
    I2CBRG = 2;
}

/****************************************************************************/
/* @brief: ESTA FUNCI?N MANDA LA SECUENCIA DE COMANDOS PARA CONFIGURAR EL   */ 
/*         SENSOR SHT3X-DIS EN MODO SINGLE SHOT PARA ESTO SE DEBE MANDAR    */
/*         LA DIRECCI?N DEL ESCLAVO JUNTO CON EL BIT DE ESCRITURA Y EL      */
/*         COMANDO DE CONFIGURACI?N.                                        */
/*           DIRECCI?N: 0X44-(0100 0100)                                    */
/*           BIT: 0-ESCRITURA | 1-LECTURA                                   */
/*           COMANDO: 0X2C0D                                                */
/*             0X2C-CLOCK STRETCHING ENABLED                                */
/*             0X0D-REPEATIBILITY MEDIUM                                    */  
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
unsigned char configurar_sensor( void ){
    iniciar_I2C();
    
    //0X88 (100 0100 0)- ES LA DIRECCI?N DEL SENSOR JUNTO EL BIT DE ESCRITURA
    enviar_dato_I2C(0X88);
    if( I2CSTATbits.ACKSTAT == 1 ) //ACK del sensor
        return NANCK;
    enviar_dato_I2C(0X2C);
    if( I2CSTATbits.ACKSTAT == 1 ) //ACK del sensor
        return NANCK;                        
    enviar_dato_I2C(0X0D);
    if( I2CSTATbits.ACKSTAT == 1 ) //ACK del sensor
        return NANCK;
    detener_I2C();
    
    return ACK;
}

/****************************************************************************/
/* @brief: ESTA FUNCI?N MANDA EL COMANDO DE I2C AL SENSOR PARA OBTENER LOS  */
/*         DATOS SENSADOS DE TEMPERATURA Y HUMEDAD. DESPUES LOS MANDA A UNA */
/*         COMPUTADORA MEDIANTE UART2                                       */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
unsigned char realizar_lectura(void){ 
    temperatura = 0;
    humedad = 0;
    iniciar_I2C();
    //0X889 (100 0100 1)- ES LA DIRECCION DEL SENSOR JUNTO EL BIT DE LECTURA
    enviar_dato_I2C(0X89);
    if( I2CSTATbits.ACKSTAT == 1 ) //ACK del sensor
        return NANCK;                        
    
    temperatura = recibe_dato_I2C();
    ack_I2C();
    temperatura = temperatura<<8 | recibe_dato_I2C();
    ack_I2C();
    
    humedad = recibe_dato_I2C();
    ack_I2C();
    humedad = humedad<<8 | recibe_dato_I2C();
    nack_I2C();
    detener_I2C();

    retardo_100ms();
    
   return ACK;
}
