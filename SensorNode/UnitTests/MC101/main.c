/**@brief: Este programa muestra el uso del convertidor Anal?gico Digital (ADC)
 * usando el sistema de interrupciones para realizar el monitoreo de una se?al
 * de pulso cardiaco mediante la computadora.
 * @device: DSPIC30F4013
 * @oscillator: FRC, 7.3728MHz
 */
#include "p30F4013.h"
/********************************************************************************/
/* 						BITS DE CONFIGURACI?N		*/	
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
/* SE DESACTIVA EL WATCHDOG														*/
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
#pragma config BODENV = BORV20           // Brown Out Voltage (2.7V)
#pragma config BOREN  = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE  = MCLR_EN          // Master Clear Enable (Enabled)
/********************************************************************************/
/*SE DESACTIVA EL C?DIGO DE PROTECCI?N						*/
/********************************************************************************/
//_FGS(CODE_PROT_OFF);      
// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

/********************************************************************************/
/* SECCI?N DE DECLARACI?N DE CONSTANTES CON DEFINE				*/
/********************************************************************************/
#define EVER 1
#define MUESTRAS 64

/********************************************************************************/
/* DECLARACIONES GLOBALES														*/
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

void iniPerifericos( void );
void iniTimer3( void );
void iniADC( void );
void iniUART1( void );
void iniInterrupciones( void );

int main( void ){
    iniPerifericos();
    iniUART1();
    iniADC();
    iniTimer3();
    iniInterrupciones();
    T3CONbits.TON = 1;     //Encendiendo TIMER3
    ADCON1bits.ADON = 1;   //Encendiendo ADC
    U2MODEbits.UARTEN = 1; //Encendiendo UART
    U2STAbits.UTXEN = 1;   //Habilitando transmisi?n
    
    for( ; EVER ; ){
        asm("PWRSAV #1");  //Modo IDLE
    }
    
    return 0;
}

/****************************************************************************/
/* DESCRICION:	ESTA RUTINA INICIALIZA LAS INTERRUPCIONES		    */
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO						    	    */
/****************************************************************************/
void iniInterrupciones( void ){
    IFS0bits.ADIF = 0;
    IFS0bits.T3IF = 0;
    IEC0bits.ADIE = 1;
    IEC0bits.T3IE = 1;
}

/****************************************************************************/
/* DESCRICION:	ESTA RUTINA INICIALIZA LOS PERIFERICOS			    */
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO							    */
/****************************************************************************/
void iniPerifericos( void ){
    PORTB = 0;
    asm("nop");
    LATB = 0;
    asm("nop");
    TRISB = 0;
    asm("nop");
    TRISBbits.TRISB2 = 1;
    asm("nop");
    PORTF = 0;
    asm("nop");
    LATF = 0;
    asm("nop");
    TRISF = 0;
    asm("nop");
    TRISFbits.TRISF5 = 0;
    asm("nop");
    TRISFbits.TRISF4 = 1;
    asm("nop");
}

/****************************************************************************/
/* DESCRICION: ESTA RUTINA INICIALIZA EL UART1 CON 19200B     		    */
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO							    */
/****************************************************************************/
void iniUART1( void ){
    U2MODE = 0x0020;
    U2STA = 0x8000;
    U2BRG = 11;      // 19200 B
}

/****************************************************************************/
/* DESCRICION: ESTA RUTINA INICIALIZA EL TIMER3 CON 8192HZ     		    */
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO							    */
/****************************************************************************/
void iniTimer3( void ){
    TMR3 = 0x0000;
    T3CON = 0x0000; // K PEX CON EL TIMER EXTERNO
    PR3 = 225;
}

/****************************************************************************/
/* DESCRICION: ESTA RUTINA INICIALIZA EL ADC                 		    */
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO							    */
/****************************************************************************/
void iniADC( void ){
    ADCON1 = 0x0044;
    ADCON2 = 0x003C;
    ADCON3 = 0x0F02; //  TcY * 3/2 PENDIENTE
    ADCHS  = 0x0002; //AN2
    ADPCFG = 0xFFFB;
    ADCSSL = 0x0000;
}
