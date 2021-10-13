.include "p30F4013.inc"

.GLOBAL __U1RXInterrupt


;**************************************************************************
; @brief: INTERRUPCIÓN ENCARGADA DE LEER LAS RESPUESTAS DEL MÓDULO WIFI
; ENVIADAS A TRAVÉS DE UART1 Y RETORNARLAS HACIA EL MÓDULO FT232 A 
; TRAVÉS DE UART2
; @params: NINGUNO
;  @RETURN: NINGUNO
;**************************************************************************
__U1RXInterrupt:
    PUSH    W0
    
    MOV	    U1RXREG,    W0
    MOV.B   WREG,	U2TXREG
    
    BCLR    IFS0,	#U1RXIF
    
    POP	    W0
    RETFIE
    
    