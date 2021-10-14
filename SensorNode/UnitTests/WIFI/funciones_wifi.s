.include "p30F4013.inc"
.GLOBAL _comandoAT

;******************************************************************************    
; @brief: ESTA RUTINA ENVIA LOS COMANDOS AT AL MODULO WIFI MEDIANTE UART1
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************
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

