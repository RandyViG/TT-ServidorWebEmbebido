.include "p30F4013.inc"
.GLOBAL _comandoAT
.global _reiniciar_SHT
    
.equ	RST,	RD0

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

    BCLR    IFS1,   #U2TXIF
    MOV	    W1,	    U2TXREG
    NOP
ENVIANDO:
    BTSS    IFS1,   #U2TXIF 
    GOTO    ENVIANDO
    
    GOTO    LEER_CMD
    
FIN:
    POP	    W1
    POP	    W0
    RETURN

;******************************************************************************    
; @brief: ESTA RUTINA RESETEA EL SENSOR CON EL OBJETIVO DE RECALIBRALO A LOS
;         VALORES POR DEFECTO Y OBTENER UN ESTADO BIEN DEFINIDO PARA EMPEZAR
;         LA RECOLECCIÓN DE LOS DATOS
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************
_reiniciar_SHT:
    BSET    PORTD,  #RST
    BCLR    PORTD,  #RST
    CALL    _retardo_15ms
    BSET    PORTD,  #RST
    
    RETURN



