/********************************************************************************
 * @brief: *ESTE PROGRAMA CONFIGURA TODOS LOS MODULOS DEL NODO SENSOR (MC101,
 * SHT3X-Dis, Wi - Fi) PARA QUE FUNCIONEN EN CONJUNTO Y OBTENGAN LAS MUESTRAS
 * DE GAS LP, HUMEDAD Y TEMPERATURA LA CUALES SERAN ENVIADAS AL SERVIDOR WEB
 * EMBEBIDO CON EL FORMATO DE LA TRAMA ESTABLECIDO.
 * NOTA:
 *   -> MÓDULO WI - FI SE CONECTA AL MIKROBUS DOS.
 *   -> SENSOR SHT3XDIS SE CONECTA AL MIKROBUS UNO.
 *   -> MC101 SE CONECTA EN LA ENTRADA ANALÓGICA DOS.
 * @device: DSPIC30F4013
 * @oscillator: FRC, 7.3728MHz 
*******************************************************************************/
#include "p30F4013.h"
#include "MC101.h"
#include "SHT3xDis.h"
#include "wifi.h"

/********************************************************************************/
/* BITS DE CONFIGURACION							*/	
/********************************************************************************/
/* SE DESACTIVA EL CLOCK SWITCHING Y EL FAIL-SAFE CLOCK MONITOR (FSCM) Y SE 	*/
/* ACTIVA EL OSCILADOR INTERNO (FAST RC) PARA TRABAJAR				*/
/* FSCM: PERMITE AL DISPOSITIVO CONTINUAR OPERANDO AUN CUANDO OCURRA UNA FALLA 	*/
/* EN EL OSCILADOR. CUANDO OCURRE UNA FALLA EN EL OSCILADOR SE GENERA UNA 	*/
/* TRAMPA Y SE CAMBIA EL RELOJ AL OSCILADOR FRC  				*/
/********************************************************************************/
//_FOSC(CSW_FSCM_OFF & FRC); 
#pragma config FOSFPR = FRC             
// Oscillator (Internal Fast RC (No change to Primary Osc Mode bits))
#pragma config FCKSMEN = CSW_FSCM_OFF   
// Clock Switching and Monitor (Sw Disabled, Mon Disabled)

/********************************************************************************/
/* SE DESACTIVA EL WATCHDOG							*/
/********************************************************************************/
//_FWDT(WDT_OFF); 
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

/********************************************************************************/
/* SE ACTIVA EL POWER ON RESET (POR), BROWN OUT RESET (BOR), 			*/	
/* POWER UP TIMER (PWRT) Y EL MASTER CLEAR (MCLR)				*/
/* POR: AL MOMENTO DE ALIMENTAR EL DSPIC OCURRE UN RESET CUANDO EL VOLTAJE DE 	*/	
/* ALIMENTACI?N ALCANZA UN VOLTAJE DE UMBRAL (VPOR), EL CUAL ES 1.85V		*/
/* BOR: ESTE MODULO GENERA UN RESET CUANDO EL VOLTAJE DE ALIMENTACI?N DECAE	*/
/* POR DEBAJO DE UN CIERTO UMBRAL ESTABLECIDO (2.7V) 				*/
/* PWRT: MANTIENE AL DSPIC EN RESET POR UN CIERTO TIEMPO ESTABLECIDO, ESTO 	*/
/* AYUDA A ASEGURAR QUE EL VOLTAJE DE ALIMENTACI?N SE HA ESTABILIZADO (16ms) 	*/
/********************************************************************************/
//_FBORPOR( PBOR_ON & BORV27 & PWRT_16 & MCLR_EN ); 
// FBORPOR
#pragma config FPWRT  = PWRT_16          // POR Timer Value (16ms)
#pragma config BODENV = BORV20           // Brown Out Voltage (2.0V)
#pragma config BOREN  = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE  = MCLR_EN          // Master Clear Enable (Enabled)

/********************************************************************************/
/*SE DESACTIVA EL C?DIGO DE PROTECCI?N						*/
/********************************************************************************/
//_FGS(CODE_PROT_OFF);      
// FGS
#pragma config GWRP = GWRP_OFF     //General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF // General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCI?N DE DECLARACION DE CONSTANTES CON DEFINE				*/
/********************************************************************************/
#define EVER 1
#define MUESTRAS 64

/********************************************************************************/
/* DECLARACIONES GLOBALES							*/
/********************************************************************************/
/*DECLARACI?N DE LA ISR DEL TIMER 1 USANDO __attribute__			*/
/********************************************************************************/
void __attribute__((__interrupt__)) _T1Interrupt( void );

/********************************************************************************/
/* CONSTANTES ALMACENADAS EN EL ESPACIO DE LA MEMORIA DE PROGRAMA		*/
/********************************************************************************/
int ps_coeff __attribute__ ((aligned (2), space(prog)));
/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO X DE LA MEMORIA DE DATOS		*/
/********************************************************************************/
int x_input[MUESTRAS] __attribute__ ((space(xmemory)));
/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO Y DE LA MEMORIA DE DATOS		*/
/********************************************************************************/
int y_input[MUESTRAS] __attribute__ ((space(ymemory)));
/********************************************************************************/
/* VARIABLES NO INICIALIZADAS LA MEMORIA DE DATOS CERCANA (NEAR), LOCALIZADA	*/
/* EN LOS PRIMEROS 8KB DE RAM							*/
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
    int i;
    unsigned char estado;
    
    idNodo = 0;
    idTemperatura = 0;
    idHumedad = 1;
    idGasLP = 2;
    
    iniciar_perifericos();
    iniciar_uart();
    iniciar_modulo_I2C();
    iniciar_adc();
    iniciar_timer3();
    iniciar_interrupciones();
    habilitar_modulos();
    
    reiniciar_SHT();
    retardo_100ms();
    
    iniciar_wifi();
    configurar_wifi();
    
    iniciar_interrupciones();
    habilitar_modulos();
    
    for( i = 0; i < 50 ; i++ ){
        configurar_sensor();
        realizar_lectura();
    }
        
    for( ; EVER ; ){
        if( bandera ){
            estado = configurar_sensor(); //SHT3xDis
            if( estado )
              continue;
            estado = realizar_lectura();
            if( estado )
               continue;
            retardo_1S();
            enviar_wifi();
            retardo_1S();
            U2TXREG = (idNodo & 0xFF00)>>8;
            U2TXREG = idNodo & 0x00FF;
            U2TXREG = idTemperatura;
            U2TXREG = (temperatura & 0xFF00)>>8;
            U2TXREG = temperatura & 0x00FF;
            retardo_1S();
            cerrar_conexion();
            
            enviar_wifi();
            retardo_1S();
            U2TXREG = (idNodo & 0xFF00)>>8;
            U2TXREG = idNodo & 0x00FF;
            U2TXREG = idHumedad;
            U2TXREG = (humedad & 0xFF00)>>8;
            U2TXREG = humedad & 0x00FF;
            retardo_1S();
            cerrar_conexion();
            
            enviar_wifi();
            retardo_1S();
            U2TXREG = (idNodo & 0xFF00)>>8;
            U2TXREG = idNodo & 0x00FF;
            U2TXREG = idGasLP;
            U2TXREG = (gasLP & 0xFF00)>>8;
            U2TXREG = gasLP & 0x00FF;
            retardo_1S();
            cerrar_conexion();
            
            bandera = 0;
            retardo_1S();
        }
        asm("nop");   
    }
   
   return 0;
}

/****************************************************************************/
/* @brief: ESTA FUNCI?N INICIALIZA LOS PERIFERICOS DEL MICROCONTROLADOR     */
/*         NECESARIOS PARA LA COMUNICACI?N CON EL SENSOR MEDIANTE UART1,    */
/*         PARA LA COMUNICACI?N MEDIANTE UART2 Y PARA EL ENVIO DE LAS       */
/*         SEÑALES DE RESET Y ENABLE AL MODULO                              */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
/****************************************************************************/
void iniciar_perifericos( void ){
    
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
    
    // UART
    PORTF = 0;
    asm("nop");
    LATF = 0;
    asm("nop");
    TRISF = 0;
    asm("nop");
    
    //UART1 - Alterno
    TRISCbits.TRISC14 = 1;   //UA1RX-RF2
    asm("nop");
    TRISCbits.TRISC13 = 0;   //UA1TX-RF3
    asm("nop");
    
    // UART2
    TRISFbits.TRISF4 = 1;   //U2Tx-RF4
    asm("nop");
    TRISFbits.TRISF5 = 0;   //U2Rx-RF4
    asm("nop");
    
    //CS - ENABLE PARA EL WIFI
    TRISBbits.TRISB8 = 0;
    asm("nop");
    
    //RESET PARA EL WIFI
    TRISDbits.TRISD1 = 0;
    asm("nop");
    
    //RESET SHT3x-DIS
    TRISDbits.TRISD0 = 0;
    asm("nop");
    //TIMER
    TRISDbits.TRISD3 = 0;
    asm("nop");
}

void iniciar_interrupciones( void ){
    // ADC
    IFS0bits.ADIF = 0;
    IEC0bits.ADIE = 1;
    // Timer3
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    // UART2-Rx
    IFS1bits.U2RXIF = 0;
    IEC1bits.U2RXIE = 1;
}

void habilitar_modulos( void ){
    T3CONbits.TON = 1;     //Encendiendo TIMER3
    ADCON1bits.ADON = 1;   //Encendiendo ADC
    U1MODEbits.UARTEN = 1; //Encendiendo UART1
    U1STAbits.UTXEN = 1;   //Habilitando transmisions
    U2MODEbits.UARTEN = 1; //Encendiendo UART2
    U2STAbits.UTXEN = 1;   //Habilitando transmision
    I2CCONbits.I2CEN = 1;  //Habilitando I2C
}
