/********************************************************************************
 *@brief: 
 * 
 * @device: DSPIC30F4013
 * @oscillator: HS, 14.7456MHz
 *******************************************************************************/
#include "p30F4013.h"
#include "MC101.h"
#include "SHT3xDis.h"
#include "wifi.h"

/********************************************************************************/
/* 						BITS DE CONFIGURACI?N									*/	
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
#pragma config BODENV = BORV20           // Brown Out Voltage (2.0V)
#pragma config BOREN  = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE  = MCLR_EN          // Master Clear Enable (Enabled)

/********************************************************************************/
/*SE DESACTIVA EL C?DIGO DE PROTECCI?N											*/
/********************************************************************************/
//_FGS(CODE_PROT_OFF);      
// FGS
#pragma config GWRP = GWRP_OFF     //General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF // General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCI?N DE DECLARACI?N DE CONSTANTES CON DEFINE								*/
/********************************************************************************/
#define EVER 1
#define MUESTRAS 64

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

/********************************************************************************/
/* DECLARACION DE FUNCIONES EN ENSAMBLADOR                                      */
/********************************************************************************/

/*FUNCIONES PARA I2C*/
void ack_I2C(void);
void nack_I2C(void);
void iniciar_I2C( void );
void detener_I2C(void);
void enviar_dato_I2C( unsigned char dato );
unsigned char recibe_dato_I2C(void);

/*FUNCIONES PARA WIFI*/
void comandoAT(unsigned char *);

void reiniciar_SHT( void );

/*FUNCIONES DE RETARDOS*/
void retardo_100ms(void);
void retardo_15ms(void);
void retardo_20ms(void);
void retardo_1S(void);

/********************************************************************************/
/* DECLARACION DE FUNCIONES PARA CONFIGURACION                                  */
/********************************************************************************/
void iniciar_perifericos( void );
void iniciar_interrupciones( void );
void habilitar_modulos( void );

/********************************************************************************/
/* DECLARACION DE VARIABLES                                                     */
/********************************************************************************/
unsigned short int humedad = 0, temperatura = 0, gasLP = 0, bandera = 0; 
unsigned short int idNodo;
unsigned char idHumedad, idTemperatura, idGasLP;

int main( void ){
    unsigned char estado;
    
    idNodo = 0;
    idTemperatura=0;
    idHumedad=1;
    idGasLP=2;
    
    iniciar_perifericos(); //OK
    iniciar_uart(); //OK
    iniciar_modulo_I2C(); //OK
    iniciar_adc(); //OK
    iniciar_timer3(); //OK
    iniciar_interrupciones(); //OK
    habilitar_modulos(); //OK
    
    reiniciar_SHT(); //OK
    retardo_100ms(); //OK
    
    iniciar_wifi();//OK
    configurar_wifi();//OK
    
    configurar_sensor(); //SHT3xDis
        
    for( ; EVER ; ){
        if( bandera ){
           estado = realizar_lectura();
            if( estado )
               continue;
            
            enviar_wifi();
            U1TXREG = (idNodo & 0xFF00)>>8;
            U1TXREG = idNodo & 0x00FF;
            U1TXREG = idTemperatura;
            U1TXREG = (temperatura & 0xFF00)>>8;
            U1TXREG = temperatura & 0x00FF;
            retardo_1S();
            cerrar_conexion();
            
            enviar_wifi();
            U1TXREG = (idNodo & 0xFF00)>>8;
            U1TXREG = idNodo & 0x00FF;
            U1TXREG = idHumedad;
            U1TXREG = (humedad & 0xFF00)>>8;
            U1TXREG = humedad & 0x00FF;
            retardo_1S();
            cerrar_conexion();
            
            
            enviar_wifi();
            U1TXREG = (idNodo & 0xFF00)>>8;
            U1TXREG = idNodo & 0x00FF;
            U1TXREG = idGasLP;
            U1TXREG = (gasLP & 0xFF00)>>8;
            U1TXREG = gasLP & 0x00FF;
            retardo_1S();
            cerrar_conexion();
            
            bandera = 0;
        }
        asm("nop");   
    }
   
   return 0;
}

/****************************************************************************/
/* @brief: ESTA FUNCI?N INICIALIZA LOS PERIFERICOS DEL MICROCONTROLADOR     */
/*         NECESARIOS PARA LA COMUNICACI?N CON EL SENSOR MEDIANTE UART1,    */
/*         PARA LA COMUNICACI?N MEDIANTE UART2 Y PARA EL ENVIO DE LAS       */
/*         SE?ALES DE RESET Y ENABLE AL MODULO                              */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO															*/
/****************************************************************************/
void iniciar_perifericos( void ){
    //Modulo Wi-Fi
    PORTA = 0;
    asm("nop");
    LATA = 0;
    asm("nop");
    TRISA = 0;
    asm("nop");
    
    //ADC
    PORTB = 0;
    asm("nop");
    LATB = 0;
    asm("nop");
    TRISB = 0;
    asm("nop");
    
    // Entrada Analogica AN2
    TRISBbits.TRISB2 = 1;
    asm("nop");
    
    PORTD = 0;
    asm("nop");
    LATD = 0;
    asm("nop");
    TRISD = 0;
    asm("nop");
    
    // UARTs
    PORTF = 0;
    asm("nop");
    LATF = 0;
    asm("nop");
    TRISF = 0;
    asm("nop");
    
    //UART1
    TRISFbits.TRISF2 = 1;   //U1RX-RF2
    asm("nop");
    TRISFbits.TRISF3 = 0;   //U1TX-RF3
    asm("nop");
    
    // UART2
    TRISFbits.TRISF4 = 1;   //U2Tx-RF4
    asm("nop");
    TRISFbits.TRISF5 = 0;   //U2Rx-RF4
    asm("nop");
    
    //CS - ENABLE PARA EL WIFI
    TRISAbits.TRISA11 = 0;
    asm("nop");
    
    //RESET PARA EL WIFI
    TRISDbits.TRISD0 = 0;
    asm("nop");    
}

void iniciar_interrupciones( void ){
    // ADC
    IFS0bits.ADIF = 0;
    IEC0bits.ADIE = 1;
    // Timer3
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    // UART1-Rx
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;
}

void habilitar_modulos( void ){
    T3CONbits.TON = 1;     //Encendiendo TIMER3
    ADCON1bits.ADON = 1;   //Encendiendo ADC
    U2MODEbits.UARTEN = 1; //Encendiendo UART2
    U2STAbits.UTXEN = 1;   //Habilitando transmision
    U1MODEbits.UARTEN = 1; //Encendiendo UART1
    U1STAbits.UTXEN   = 1; //Habilitando transmisions
}