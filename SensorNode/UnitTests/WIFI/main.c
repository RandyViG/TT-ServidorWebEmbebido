/********************************************************************************
 * @brief: ESTE PROGRAMA CONFIGURA LOS PUERTOS DEL MICROCONTROLADOR Y EL MODO 
 * DE OPERACIÓN DEL MÓDULO DE COMUNICACIÓN ESP8266 PARA EL ENVÍO DE DATOS 
 * DESDE AL MICROCONTROLADOR HACÍA OTRO DISPOSITIVO USANDO WIFI
 * @device: DSPIC30F4013
 * @oscillator: FRC, 7.3728MHz
 *******************************************************************************/
#include "p30F4013.h"
#include <stdio.h>
#include <stdlib.h>

/********************************************************************************
* 					         	 BITS DE CONFIGURACIÓN  									  *	
*********************************************************************************
* SE DESACTIVA EL CLOCK SWITCHING Y EL FAIL-SAFE CLOCK MONITOR (FSCM) Y SE 	
* ACTIVA EL OSCILADOR INTERNO (FAST RC) PARA TRABAJAR							
* FSCM: PERMITE AL DISPOSITIVO CONTINUAR OPERANDO AUN CUANDO OCURRA UNA FALLA 	
* EN EL OSCILADOR. CUANDO OCURRE UNA FALLA EN EL OSCILADOR SE GENERA UNA 		
* TRAMPA Y SE CAMBIA EL RELOJ AL OSCILADOR FRC  								
********************************************************************************/
//_FOSC(CSW_FSCM_OFF & FRC); 
#pragma config FOSFPR = FRC             
// Oscillator (Internal Fast RC (No change to Primary Osc Mode bits))
#pragma config FCKSMEN = CSW_FSCM_OFF   
// Clock Switching and Monitor (Sw Disabled, Mon Disabled)

/********************************************************************************/
/* SE DESACTIVA EL WATCHDOG														           */
/********************************************************************************/
//_FWDT(WDT_OFF); 
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

/********************************************************************************
* SE ACTIVA EL POWER ON RESET (POR), BROWN OUT RESET (BOR), 					   
* POWER UP TIMER (PWRT) Y EL MASTER CLEAR (MCLR)								
* POR: AL MOMENTO DE ALIMENTAR EL DSPIC OCURRE UN RESET CUANDO EL VOLTAJE DE 	
* ALIMENTACIÓN ALCANZA UN VOLTAJE DE UMBRAL (VPOR), EL CUAL ES 1.85V			
* BOR: ESTE MODULO GENERA UN RESET CUANDO EL VOLTAJE DE ALIMENTACIÓN DECAE		
* POR DEBAJO DE UN CIERTO UMBRAL ESTABLECIDO (2.7V) 							
* PWRT: MANTIENE AL DSPIC EN RESET POR UN CIERTO TIEMPO ESTABLECIDO, ESTO 		
* AYUDA A ASEGURAR QUE EL VOLTAJE DE ALIMENTACIÓN SE HA ESTABILIZADO (16ms) 	
********************************************************************************/
//_FBORPOR( PBOR_ON & BORV27 & PWRT_16 & MCLR_EN ); 
// FBORPOR
#pragma config FPWRT  = PWRT_16          // POR Timer Value (16ms)
#pragma config BODENV = BORV20           // Brown Out Voltage (2.7V)
#pragma config BOREN  = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE  = MCLR_EN          // Master Clear Enable (Enabled)

/*********************************************************************************/
/*SE DESACTIVA EL CÓDIGO DE PROTECCIÓN										            	*/
/*********************************************************************************/
//_FGS(CODE_PROT_OFF);      
// FGS
#pragma config GWRP = GWRP_OFF     //General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF //General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCIÓN DE DECLARACIÓN DE CONSTANTES CON DEFINE								        */
/********************************************************************************/
#define MUESTRAS 64
#define EVER 1
#define NANCK 1
#define ACK 0

/********************************************************************************/
/* DECLARACIONES GLOBALES														              */
/********************************************************************************/
/*DECLARACIÓN DE LA ISR DEL TIMER 1 USANDO __attribute__						        */
/********************************************************************************/
void __attribute__((__interrupt__)) _T1Interrupt( void );

/********************************************************************************/
/* CONSTANTES ALMACENADAS EN EL ESPACIO DE LA MEMORIA DE PROGRAMA				     */
/********************************************************************************/
int ps_coeff __attribute__ ((aligned (2), space(prog)));

/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO X DE LA MEMORIA DE DATOS			     */
/********************************************************************************/
int x_input[MUESTRAS] __attribute__ ((space(xmemory)));

/********************************************************************************/
/* VARIABLES NO INICIALIZADAS EN EL ESPACIO Y DE LA MEMORIA DE DATOS			     */
/********************************************************************************/
int y_input[MUESTRAS] __attribute__ ((space(ymemory)));

/********************************************************************************/
/* VARIABLES NO INICIALIZADAS LA MEMORIA DE DATOS CERCANA (NEAR), LOCALIZADA	  */
/* EN LOS PRIMEROS 8KB DE RAM													              */
/********************************************************************************/
int var1 __attribute__ ((near));

/********************************************************************************
 * DECLARACIÓN DE FUNCIONES
 ********************************************************************************/

/*FUNCIONES PARA CONFIGURACIÓN*/
void configPuertos      ( void );
void configUART         ( void );
void configWIFI         ( void );

/*FUNCIONES PARA MÓDULO WIFI*/
void activaUART         ( void );
void iniInterrupciones  ( void );
void iniWIFI            ( void );
void comandoAT          (unsigned char *);
void RETARDO_1S         ( void );
void cerrarConexion     ( void );

/*COMANDOS AT ÚTILES PARA LA CONFIGURACIÓN DEL SENSOR*/
unsigned char cmdRST[] = "AT+RST\r\n";
unsigned char cmdCWMODE[] = "AT+CWMODE=3\r\n";//Original Mode : 1
unsigned char cmdCIPMUX[] = "AT+CIPMUX=0\r\n"; // ????
unsigned char cmdCWJAP[] = "AT+CWJAP=\"SSID\",\"PASSWORD\"\r\n";
unsigned char cmdCIFSR[] = "AT+CIFSR\r\n";
unsigned char cmdCIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.0.125\",5000\r\n";
unsigned char cmdCIPMODE[] = "AT+CIPMODE=1\r\n";
unsigned char cmdCIPSEND[] = "AT+CIPSEND\r\n";
unsigned char cmdCIPCLOSE[] = "AT+CIPCLOSE\r\n";
unsigned char cmdSTOPPT[] = "+++";

int main (void)
{
   configPuertos();
   configUART();
   
   iniInterrupciones();
   activaUART();
 
   iniWIFI();
   configWIFI();
   
   U1TXREG = 'H';
   U1TXREG = '0';
   U1TXREG = 'L';
   U1TXREG = 'A';
   
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   
   cerrarConexion();
   
   for(;EVER;)
   {
      asm("nop");   
   }
   return 0;
}

/******************************************************************
* @brief: ESTA RUTINA CIERRA LA CONEXION TCP. PRIMERO SE DETIENE 
* EL MODO "passthrough" AL ENVIAR LA CADENA "+++". AL FINAL SE
* BORRA LA CONEXION TCP.
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void cerrarConexion( void )
{
   comandoAT(cmdSTOPPT);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   comandoAT(cmdCIPCLOSE);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
}

 /******************************************************************
* @brief: ESTA RUTINA INICIALIZA EL MÓDULO ESP8266 MANDANDO UN PULSO
* CUADRADO AL BIT RST E INICIALIZANDO CHPD EN 1
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void iniWIFI( void )
{
   PORTAbits.RA11 = 1;
   asm("nop");
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   PORTDbits.RD0 = 1;
   asm("nop");
   RETARDO_1S();
   PORTDbits.RD0 = 0;
   asm("nop");
   RETARDO_1S();
   PORTDbits.RD0 = 1;
   asm("nop");
   RETARDO_1S();
}

/******************************************************************
* @brief: ESTA RUTINA INICIALIZA LAS INTERRUPCIONES
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void iniInterrupciones( void )
{
// SE HABILITA LA INTERRUPCION DE RECEPCION DEL UART1
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;
}

/******************************************************************
* @brief: ESTA RUTINA HABILITA LOS UART 1 Y 2 DEL MICROCONTROLADOR
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void activaUART( void )
{
   U2MODEbits.UARTEN = 1;
   U2STAbits.UTXEN   = 1;
   
   U1MODEbits.UARTEN = 1;
   U1STAbits.UTXEN   = 1;       
}

/******************************************************************
* @brief: ESTA RUTINA CONFIGURA EL MÓDULO WIFI MEDIANTE EL USO DE
* LOS COMANDOS AT
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void configWIFI( void )
{
   comandoAT(cmdRST);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();   

   comandoAT(cmdCWMODE);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();

   comandoAT(cmdCWJAP);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();   
   RETARDO_1S();
   RETARDO_1S();

   comandoAT(cmdCIPSTART);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();

   // SE CONFIGURA MODO "passthrough"
   comandoAT(cmdCIPMODE);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();

   comandoAT(cmdCIPSEND);
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
   RETARDO_1S();
}

/******************************************************************
* @brief: ESTA RUTINA INICIALIZA EL UART1 Y UART2 CON UNA 
* VELOCIDAD DE 115200 BAUDIOS
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void configUART( void )
{
   U1MODE = 0x0000;
   U1STA  = 0x8000;         
   U1BRG  = 1;
   
   U2MODE = 0x0000;
   U2STA  = 0x8000;   
   U2BRG  = 11;   
}

/******************************************************************
* @brief: ESTA RUTINA CONFIGURA LOS PUESTOS DEL MICROCONTROLADOR
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void configPuertos( void )
{
    PORTA = 0;
    asm("nop");
    LATA = 0;
    asm("nop");
    TRISA = 0;
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

    //Receptor y Transmisor de UART1
    TRISFbits.TRISF2 = 1;   //U1RX-RF2
    asm("nop");
    TRISFbits.TRISF3 = 0;   //U1TX-RF3
    asm("nop");
    //Receptor y Transmisor de UART2
    TRISFbits.TRISF4 = 1;   //U2RX-RF4
    asm("nop");
    TRISFbits.TRISF5 = 0;   //U2TX-RF5
    asm("nop");
    //CS para el módulo WIFI
    TRISAbits.TRISA11 = 0;
    asm("nop");
    //Reset para el módulo WIFI
    TRISDbits.TRISD0 = 0;
    asm("nop");
      
}