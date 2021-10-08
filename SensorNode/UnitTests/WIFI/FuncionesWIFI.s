.include "p30F4013.inc"

.GLOBAL _comandoAT

;***************************************************************
; @brief: ESTA RUTINA SE ENCARGA DE EL ENVÍO DE UN COMANDO AT
; HACIA EL MÓDULO WI-FI. OBTIENE LA CADENA DEL COMANDO AT Y ES
; ENVIADO CARACTER POR CARACTER A TRAVÉS DE UART1.
; @param: DIRECCIÓN DE MEMORIA DEL COMANDO AT ALMACENADO EN W0
; @return: Ninguno
;***************************************************************	

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

    





