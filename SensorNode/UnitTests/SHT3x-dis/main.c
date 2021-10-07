/********************************************************************************
 * @brief: ESTE PROGRAMA CONFIGURA LOS PUERTOS DEL MICROCONTROLADOR Y EL SENSOR 
 *     SHT3x-dis EN MODO SINGLE-SHOT PARA OBTENER LAS MEDICIONES DE TEMPERATURA 
 *     Y HUMEDAD LAS CUALES SERAN ENVIADAS A UNA PC MEDIANTE LA INTERFAZ UART2.
 * @device: DSPIC30F4013
 * @oscillator: FRC, 7.3728MHz
 *******************************************************************************/
#include "p30F4013.h"
#include <stdio.h>
#include <stdlib.h>

/********************************************************************************/
/* 						BITS DE CONFIGURACIÓN									*/	
/********************************************************************************/
/* SE DESACTIVA EL CLOCK SWITCHING Y EL FAIL-SAFE CLOCK MONITOR (FSCM) Y SE 	*/
/* ACTIVA EL OSCILADOR INTERNO (FAST RC) PARA TRABAJAR							*/
/* FSCM: PERMITE AL DISPOSITIVO CONTINUAR OPERANDO AUN CUANDO OCURRA UNA FALLA 	*/
/* EN EL OSCILADOR. CUANDO OCURRE UNA FALLA EN EL OSCILADOR SE GENERA UNA 		*/
/* TRAMPA Y SE CAMBIA EL RELOJ AL OSCILADOR FRC  								*/
/********************************************************************************/
//_FOSC(CSW_FSCM_OFF & FRC); 
#pragma config FOSFPR = FRC             
// Oscillator (Internal Fast RC (No change to Primary Osc Mode bits))
#pragma config FCKSMEN = CSW_FSCM_OFF   
// Clock Switching and Monitor (Sw Disabled, Mon Disabled)

/********************************************************************************/
/* SE DESACTIVA EL WATCHDOG														*/
/********************************************************************************/
//_FWDT(WDT_OFF); 
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

/********************************************************************************/
/* SE ACTIVA EL POWER ON RESET (POR), BROWN OUT RESET (BOR), 					*/	
/* POWER UP TIMER (PWRT) Y EL MASTER CLEAR (MCLR)								*/
/* POR: AL MOMENTO DE ALIMENTAR EL DSPIC OCURRE UN RESET CUANDO EL VOLTAJE DE 	*/	
/* ALIMENTACI?N ALCANZA UN VOLTAJE DE UMBRAL (VPOR), EL CUAL ES 1.85V			*/
/* BOR: ESTE MODULO GENERA UN RESET CUANDO EL VOLTAJE DE ALIMENTACI?N DECAE		*/
/* POR DEBAJO DE UN CIERTO UMBRAL ESTABLECIDO (2.7V) 							*/
/* PWRT: MANTIENE AL DSPIC EN RESET POR UN CIERTO TIEMPO ESTABLECIDO, ESTO 		*/
/* AYUDA A ASEGURAR QUE EL VOLTAJE DE ALIMENTACI?N SE HA ESTABILIZADO (16ms) 	*/
/********************************************************************************/
//_FBORPOR( PBOR_ON & BORV27 & PWRT_16 & MCLR_EN ); 
// FBORPOR
#pragma config FPWRT  = PWRT_16          // POR Timer Value (16ms)
#pragma config BODENV = BORV20           // Brown Out Voltage (2.7V)
#pragma config BOREN  = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE  = MCLR_EN          // Master Clear Enable (Enabled)

/********************************************************************************/
/*SE DESACTIVA EL C?DIGO DE PROTECCI?N											*/
/********************************************************************************/
//_FGS(CODE_PROT_OFF);      
// FGS
#pragma config GWRP = GWRP_OFF     //General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF //General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCIÓN DE DECLARACIÓN DE CONSTANTES CON DEFINE								*/
/********************************************************************************/
#define MUESTRAS 64
#define EVER 1
#define NANCK 1
#define ACK 0

/********************************************************************************/
/* DECLARACIONES GLOBALES														*/
/********************************************************************************/
/*DECLARACI?N DE LA ISR DEL TIMER 1 USANDO __attribute__						*/
/********************************************************************************/
void __attribute__((__interrupt__)) _T1Interrupt( void );

/********************************************************************************/
/* CONSTANTES ALMACENADAS EN EL ESPACIO DE LA MEMORIA DE PROGRAMA				*/
/********************************************************************************/
int ps_coeff __attribute__ ((aligned (2), space(prog)));

/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO X DE LA MEMORIA DE DATOS			*/
/********************************************************************************/
int x_input[MUESTRAS] __attribute__ ((space(xmemory)));

/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO Y DE LA MEMORIA DE DATOS			*/
/********************************************************************************/
int y_input[MUESTRAS] __attribute__ ((space(ymemory)));

/********************************************************************************/
/* VARIABLES NO INICIALIZADAS LA MEMORIA DE DATOS CERCANA (NEAR), LOCALIZADA	*/
/* EN LOS PRIMEROS 8KB DE RAM													*/
/********************************************************************************/
int var1 __attribute__ ((near));

/********************************************************************************
 * DECLARACIÓN DE FUNCIONES
 ********************************************************************************/
/*FUNCIONES PARA CONFIGURACIÓN*/
void iniciar_perifericos( void );
void iniciar_modulo_uart2( void );
void iniciar_modulo_I2C( void );

/*FUNCIONES PARA SENSOR SHT*/
void reiniciar_SHT(void);
unsigned char configurar_sensor(void);
unsigned char realizar_lectura(void);

/*FUNCIONES PARA I2C*/
void iniciar_I2C( void );
void detener_I2C(void);
void enviar_dato_I2C( unsigned char dato );
unsigned char recibe_dato_I2C(void);
void ack_I2C(void);
void nack_I2C(void);

/*FUNCIONES DE RETARDOS*/
void retardo_100ms(void);
void retardo_15ms(void);
void retardo_20ms(void);
void retardo_1S(void);

void envia_dato_PC( unsigned int cmd );

int main( void ){
    unsigned char estado;

    iniciar_perifericos();
    
    iniciar_modulo_uart2();
    iniciar_modulo_I2C();
    
    /*SE HABILITA EL UART2*/
    U2MODEbits.UARTEN = 1;
    /*SE HABILITA LA TRANSMISIÓN*/
    U2STAbits.UTXEN = 1;
    
    reiniciar_SHT();
    
    retardo_100ms();
    
    for( ; EVER ; ){
        estado = configurar_sensor();
        if( estado )
            continue;
        estado = realizar_lectura();
        if( estado )
            continue;
        retardo_1S();
        retardo_1S();     
    }
    return 0;
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN INICIALIZA LOS PERIFERICOS DEL MICROCONTROLADOR     */
/*         NECESARIOS PARA LA COMUNICACIÓN CON EL SENSOR MEDIANTE 12C.       */
/*         LIMPIA EL PUERTO F, LO ESTABLE COMO SALIDA (OUTPUT), CONFIGURA   */
/*         UART2 PONE EL PIN RF4 (UT2RX) COMO INPUT Y EL PIN RF5 (U2TX)     */
/*         COMO OUTPUT                                                       */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
void iniciar_perifericos( void ){
    PORTF = 0;
    asm("nop");
    LATF = 0;
    asm("nop");
    TRISF = 0;
    asm("nop");
        
    PORTFbits.RF5 = 0;
    PORTFbits.RF4 = 1;
    asm("nop");
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN INICIALIZA UART2 CON UN BAUDAGE DE 9600             */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
void iniciar_modulo_uart2( void ){
    U2MODE = 0x0020;
    U2STA = 0x8000;
    U2BRG = 11;
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN INICIALIZA I2C EN MODO FAST MODE 400KHz             */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
void iniciar_modulo_I2C( void ){
    I2CCON = 0X8000;
    I2CBRG = 2;
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN MANDA LA SECUENCIA DE COMANDOS PARA CONFIGURAR EL   */ 
/*         SENSOR SHT3X-DIS EN MODO SINGLE SHOT PARA ESTO SE DEBE MANDAR    */
/*         LA DIRECCIÓN DEL ESCLAVO JUNTO CON EL BIT DE ESCRITURA Y EL      */
/*         COMANDO DE CONFIGURACIÓN.                                        */
/*           DIRECCIÓN: 0X44-(0100 0100)                                    */
/*           BIT: 0-ESCRITURA | 1-LECTURA                                   */
/*           COMANDO: 0X2C0D                                                */
/*             0X2C-CLOCK STRETCHING ENABLED                                */
/*             0X0D-REPEATIBILITY MEDIUM                                    */  
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
unsigned char configurar_sensor( void ){
    iniciar_I2C();
    
    //0X88 (100 0100 0)- ES LA DIRECCIÓN DEL SENSOR JUNTO EL BIT DE ESCRITURA
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
/* @brief: ESTA FUNCIÓN MANDA EL COMANDO DE I2C AL SENSOR PARA OBTENER LOS  */
/*         DATOS SENSADOS DE TEMPERATURA Y HUMEDAD. DESPUES LOS MANDA A UNA */
/*         COMPUTADORA MEDIANTE UART2                                       */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
unsigned char realizar_lectura(void){
    unsigned int dato1, dato2, dato3, dato4;
    
    iniciar_I2C();
    //0X889 (100 0100 1)- ES LA DIRECCIÓN DEL SENSOR JUNTO EL BIT DE LECTURA
    enviar_dato_I2C(0X89);
    if( I2CSTATbits.ACKSTAT == 1 ) //ACK del sensor
        return NANCK;                        

    dato1 =  recibe_dato_I2C();
    ack_I2C();
    dato2 = recibe_dato_I2C();
    ack_I2C();
    dato3 = recibe_dato_I2C();
    ack_I2C();
    dato3 = recibe_dato_I2C();
    ack_I2C();
    dato4 = recibe_dato_I2C();
    nack_I2C();
    detener_I2C();
    
    //ENVIA MSB Temperatura (PARTE ALTA)
    envia_dato_PC(dato1);
    retardo_100ms();
    //ENVIA LSB Temperatura (PARTE BAJA)
    envia_dato_PC(dato2);
    retardo_100ms();
    //ENVIA MSB Humedad (PARTE ALTA)
    envia_dato_PC(dato3);
    retardo_100ms();
    //ENVIA LSB Humedad (PARTE BAJA)
    envia_dato_PC(dato4);
    retardo_100ms();
    
   return ACK;
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN MANDA UN BYTE A LA COMPUTADORA MEDIANTE UART        */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
void envia_dato_PC( unsigned int cmd ){
    IFS1bits.U2TXIF = 0;
    U2TXREG = cmd;
        while( !IFS1bits.U2TXIF );
    asm("nop");
}