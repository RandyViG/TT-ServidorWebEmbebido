#include"p30F4013.h"

/****************************************************************************/
/* DESCRICION: ESTA RUTINA INICIALIZA EL TIMER3 CON 1.5HZ     		    */
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO							    */
/****************************************************************************/
void iniciar_timer3( void ){
    TMR3 = 0x0000;
    PR3 = 19200;
    T3CON = 0x0000; 
}

/****************************************************************************/
/* DESCRICION: ESTA RUTINA INICIALIZA EL ADC                 		    */
/* PARAMETROS: NINGUNO                                                      */
/* RETORNO: NINGUNO							    */
/****************************************************************************/
void iniciar_adc( void ){
    ADCON1 = 0x0044;
    ADCON2 = 0x003C;
    ADCON3 = 0x0F02;
    ADCHS  = 0x0002; //AN2
    ADPCFG = 0xFFFB;
    ADCSSL = 0x0000;
}
