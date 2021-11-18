/********************************************************************************
 *@brief: ESTE PROGRAMA CONFIGURA EL SENSOR WIFI ESP8266EX PARA FUNCIONAR COMO
 *    COMO CLIENTE TCP Y ENVIAR UN MENSAJE DE ECHO AL SERVIDOR. PARA CONFIGURAR
 *    EL MODULO WIFI SE UTILIZAN COMANDOS AT LOS CUALES SE ENVIAN MEDIANTE UART2
 *    Y LAS RESPUESTAS DEL MODULO SE MANDAN A LA COMPUTADORA POR UART1A
 *    NOTA:
 *       -> EL MODULO SE CONECTA EN EL MIKROBUS2
 *       -> EL MODULO FT232 SE CONECTA AL CONECTOR PMOD3
 * 
 *  ESP8266EX         DSPI30F4013           FT232
 *  ---------       ----------------       ------
 * |         |     | UART1    UART2 |     |      | 
 * |MODULO RX|<----|TX(RF3)  (RF5)TX|---->|RX    |----> PC
 * |WIFI   TX|---->|RX(RF2)         |     |      |
 * |      RST|<----|RD0             |     |      |
 * |     CHPD|<----|RA11            |     |      |
 *  ---------       ----------------       ------
 * 
 * @device: DSPIC30F4013
 * @oscillator: HS, 14.7456MHz
 *******************************************************************************/
#include "p30F4013.h"

/********************************************************************************/
/* BITS DE CONFIGURACION							*/	
/********************************************************************************/
/* SE DESACTIVA EL CLOCK SWITCHING Y EL FAIL-SAFE CLOCK MONITOR (FSCM) Y SE 	*/
/* ACTIVA EL OSCILADOR INTERNO (FAST RC) PARA TRABAJAR	 			*/
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
/*SE DESACTIVA EL C?DIGO DE PROTECCION						*/
/********************************************************************************/
//_FGS(CODE_PROT_OFF);      
// FGS
#pragma config GWRP = GWRP_OFF     //General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF // General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCI?N DE DECLARACI?N DE CONSTANTES CON DEFINE 				*/
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

/********************************************************************************
 * DECLARACIÓN DE FUNCIONES
 ********************************************************************************/
/*FUNCIONES PARA CONFIGURACIÓN*/
void iniciar_puertos( void );
void iniciar_uart( void );
void configurar_wifi( void );
void iniciar_interrupciones( void );
void habilitar_uart( void );

/*FUNCIONES PARA MODULO WIFI - ESP8266*/
void iniciar_wifi( void );
void comandoAT(unsigned char *);
void enviar_wifi(void);
void cerrar_conexion( void );

/*FUNCIONES DE RETARDOS*/
void retardo_100ms(void);
void retardo_15ms(void);
void retardo_20ms(void);
void retardo_1S(void);

/********************************************************************************
 * DECLARACIÓN DE VARIABLES GLOBALES
 ********************************************************************************/
/*COMANDOS AT DE CONFIGURACIÓN*/
unsigned char cmdRST[] = "AT+RST\r\n";
unsigned char cmdCWMODE[] = "AT+CWMODE=1\r\n";
unsigned char cmdCIPMUX[] = "AT+CIPMUX=0\r\n";
//unsigned char cmdCWJAP[] = "AT+CWJAP=\"ssid\",\"password\"\r\n";
unsigned char cmdCWJAP[] = "AT+CWJAP=\"IZZI-6743\",\"50A5DC686743\"\r\n";
//unsigned char cmdCWJAP[] = "AT+CWJAP=\"IZZI-6893\",\"2WC468400355\"\r\n";
unsigned char cmdCIFSR[] = "AT+CIFSR\r\n";
unsigned char cmdCIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.0.14\",6000\r\n";
unsigned char cmdCIPMODE[] = "AT+CIPMODE=1\r\n";
unsigned char cmdCIPSEND[] = "AT+CIPSEND\r\n";
unsigned char cmdCIPCLOSE[] = "AT+CIPCLOSE\r\n";
unsigned char cmdSTOPPT[] = "+++";

/*VARIABLES DE SENSORES*/
unsigned short int temperatura, idNodo;
unsigned char idTemperatura;

int main (void){
    idNodo = 0;
    idTemperatura = 1;
    temperatura = 255;
    
    iniciar_puertos();
    iniciar_uart();
   
    iniciar_interrupciones();
    habilitar_uart();
 
    iniciar_wifi();
    configurar_wifi();
        
    for( ; EVER ; ){
        enviar_wifi();
      
        U2TXREG = (idNodo & 0xFF00)>>8;
        U2TXREG = idNodo & 0x00FF;
        U2TXREG = idTemperatura;
        U2TXREG = (temperatura & 0xFF00)>>8;
        U2TXREG = temperatura & 0x00FF;
        
        retardo_1S();  
        cerrar_conexion();
        asm("nop");   
    }
   
   return 0;
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN INICIALIZA LOS PERIFERICOS DEL MICROCONTROLADOR     */
/*         NECESARIOS PARA LA COMUNICACIÓN CON EL MODULO WIFI ESP8266,      */
/*         MEDIANTE UART2 Y PARA EL ENVIO DE LAS SEÑALES DE RESET Y         */
/*         ENABLE AL MODULO                                                 */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
/****************************************************************************/
void iniciar_puertos( void ){
    
    PORTB = 0;
    asm("nop");
    LATB = 0;
    asm("nop");
    TRISB = 0;
    asm("nop");
    
    PORTD = 0;
    asm("nop");
    LATD = 0;
    asm("nop");
    TRISD = 0;
    asm("nop");
            
    PORTF = 0;
    asm("nop");
    LATF = 0;
    asm("nop");
    TRISF = 0;
    asm("nop");

    //UART1    
    TRISCbits.TRISC14 = 1;   //UA1RX-RC14
    asm("nop");
    TRISCbits.TRISC13 = 0;   //UA1TX-RC13
    asm("nop");
    
    //UART2
    TRISFbits.TRISF4 = 1;   //U2RX-RF4
    asm("nop");
    TRISFbits.TRISF5 = 0;   //U2TX-RF5
    asm("nop");
    
    //CS - ENABLE PARA EL WIFI
    TRISBbits.TRISB8 = 0;
    asm("nop");
    
    //RESET PARA EL WIFI
    TRISDbits.TRISD1 = 0;
    asm("nop");
    
    ADPCFG = 0xFFFF;
    asm("nop");
    
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN CONFIGURA EL UART1 ALTERNO Y UART2 CON LA           */
/*         VELOCIDAD DE 115200 BAUDIOS                                      */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
/****************************************************************************/
void iniciar_uart( void ){
    U1MODE = 0x0420;
    U1STA  = 0x8000;         
    U1BRG  = 0;
   
    U2MODE = 0x0020;
    U2STA  = 0x8000;
    U2BRG  = 0;
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN INICIALIZA LAS INTERRUPCIONES                       */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
/****************************************************************************/
void iniciar_interrupciones( void ){
    // SE HABILITA LA INTERRUPCION RX DEL UART2
    IFS1bits.U2RXIF = 0;
    IEC1bits.U2RXIE = 1;
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN HABILITA UART1 Y UART2                              */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							                                            */
/****************************************************************************/
void habilitar_uart( void ){
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN   = 1; 
    
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN   = 1;      
}

/****************************************************************************/
/* @brief: ESTA FUNCIÓN INICIALIZA EL ESP8266 COLOCANDO UN PUSLO EN SRT Y   */
/*         HABILITANDO LA BANDERA DE ENABLE                                 */
/*                  RST --------         --------                           */
/*                              |       |                                   */
/*                              |       |                                   */
/*                              ---------                                   */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							                                            */
/****************************************************************************/
void iniciar_wifi( void ){
    PORTBbits.RB8 = 1;
    asm("nop");
    retardo_1S();
    //retardo_1S();
    //retardo_1S();
    PORTDbits.RD1 = 1;
    asm("nop");
    retardo_1S();
    PORTDbits.RD1 = 0;
    asm("nop");
    retardo_1S();
    PORTDbits.RD1 = 1;
    asm("nop");
    retardo_1S();
}

/****************************************************************************/
/* @brief: ESTA FUNCION CONFIGURA EL MODULO ESP8266 COMO CLIENTE TCP Y      */
/*         PARA EL ENVIO DE DATOS MEDIANTE UART1                            */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							                                            */
/****************************************************************************/
void configurar_wifi( void ){
    comandoAT(cmdRST);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();      
    comandoAT(cmdCWMODE);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();   
    comandoAT(cmdCIPMUX);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();   
    comandoAT(cmdCWJAP);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();       
    
}


/****************************************************************************/
/* @brief: ESTA FUNCION ESTABLECE EL INICIO DE ENVIO DE DATOS               */
/*         MEDIANTE EL MODO "PASSTHROUGH" DEL MODULO ESP8266                */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							                                            */
/****************************************************************************/
void enviar_wifi(void){
    comandoAT(cmdCIPSTART);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    
    comandoAT(cmdCIPMODE); // SE CONFIGURA MODO "passthrough"
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();  
    retardo_1S();
    
    comandoAT(cmdCIPSEND);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
}

/****************************************************************************/
/* @brief: ESTA FUNCION CIERRA LA CONEXION TCP CON EL SERVIDOR.             */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							                                            */
/****************************************************************************/
void cerrar_conexion( void ){
    comandoAT(cmdSTOPPT);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    comandoAT(cmdCIPCLOSE);
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
    retardo_1S();
}
