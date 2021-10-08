    .include "p30F4013.inc"
    .global _comandoAT

;**@brief: Esta funci�n envia los comandos AT al m�dulo WIFI.
; *	   Se usa el UART1.
; *@param: W0, tiene la direcci�n de la cadena AT
; *@return: Ninguno
	
_comandoAT:
    PUSH    W0
    PUSH    W1
    
LEER_CMD:
    MOV.B   [W0++], W1
   
    CP0.B   W1
    BRA	    Z,	    FIN

    BCLR    IFS0,   #U1TXIF
    MOV	    W1,	    U1TXREG
    NOP
ENVIANDO:
    BTSS    IFS0,   #U1TXIF 
    GOTO    ENVIANDO
    
    GOTO    LEER_CMD
    
FIN:
    POP	    W1
    POP	    W0
    RETURN

    





