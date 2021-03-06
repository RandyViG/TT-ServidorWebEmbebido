#include"p30F4013.h"

extern void comandoAT(unsigned char *);
extern void retardo_1S(void);

/********************************************************************************
 * DECLARACION DE VARIABLES GLOBALES
 ********************************************************************************/
/*COMANDOS AT DE CONFIGURACI?N*/
unsigned char cmdRST[] = "AT+RST\r\n";
unsigned char cmdECHO[] = "ATE0\r\n";
unsigned char cmdCWMODE[] = "AT+CWMODE=1\r\n";
unsigned char cmdCIPMUX[] = "AT+CIPMUX=0\r\n";
//unsigned char cmdCWJAP[] = "AT+CWJAP=\"ssid\",\"password\"\r\n";
unsigned char cmdCWJAP[] = "AT+CWJAP=\"IZZI-6743\",\"50A5DC686743\"\r\n";
//unsigned char cmdCWJAP[] = "AT+CWJAP=\"IZZI-6893\",\"2WC468400355\"\r\n";
unsigned char cmdCIFSR[] = "AT+CIFSR\r\n";
unsigned char cmdCIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.0.14\",6000\r\n";
//unsigned char cmdCIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.0.31\",6000\r\n";
unsigned char cmdCIPMODE[] = "AT+CIPMODE=1\r\n";
unsigned char cmdCIPSEND[] = "AT+CIPSEND\r\n";
unsigned char cmdCIPCLOSE[] = "AT+CIPCLOSE\r\n";
unsigned char cmdSTOPPT[] = "+++";

/****************************************************************************/
/* @brief: ESTA FUNCION CONFIGURA EL UART1 Y UART2 CON LA VELOCIDAD DE      */
/*         115200 BAUDIOS                                                   */
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
/* @brief: ESTA FUNCION INICIALIZA EL ESP8266 COLOCANDO UN PUSLO EN SRT Y   */
/*         HABILITANDO LA BANDERA DE ENABLE                                 */
/*                  RST --------         --------                           */
/*                              |       |                                   */
/*                              |       |                                   */
/*                              ---------                                   */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
/****************************************************************************/
void iniciar_wifi( void ){
    PORTBbits.RB8 = 1;
    asm("nop");
    retardo_1S();
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
/* @brief: ESTA FUNCION CONFIGURA EL MODULO ESP2866 COMO CLIENTE TCP Y      */
/*         PARA EL ENVIO DE DATOS MEDIANTE UART1                            */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
/****************************************************************************/
void configurar_wifi( void ){
    comandoAT(cmdRST);
    retardo_1S();
    retardo_1S();
    //comandoAT(cmdECHO);
    //retardo_1S();    
    //retardo_1S();
    //retardo_1S();
    comandoAT(cmdCWMODE);
    retardo_1S(); 
    retardo_1S(); 
    comandoAT(cmdCIPMUX);
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
/*         MEDIANTE EL MODO "PASSTHROUGH" DEL MODULO ESP2866                */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
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
    
    comandoAT(cmdCIPSEND);
    retardo_1S();
    retardo_1S();
    retardo_1S();
}
/****************************************************************************/
/* @brief: ESTA FUNCION CIERRA LA CONEXION TCP CON EL SERVIDOR.             */
/* @params: NINGUNO                                                         */
/* @return: NINGUNO							    */
/****************************************************************************/
void cerrar_conexion( void ){
    comandoAT(cmdSTOPPT);
    retardo_1S();
    retardo_1S(); 
    comandoAT(cmdCIPCLOSE);
    retardo_1S();
    retardo_1S();
    retardo_1S();
}
