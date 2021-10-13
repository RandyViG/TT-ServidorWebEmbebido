.include "p30F4013.inc"
.GLOBAL __U1RXInterrupt

;******************************************************************************    
; @brief: ESTA RUTINA LEE LAS RESPUESTAS DEL MODULO WIFI ENVIADAS A TRAVES
;         DE UART1 Y LAS ENVIA MEDIANTE UART2 HACIA EL MODULO FT232
; @params: NINGUNO
; @return: NINGUNO
;****************************************************************************** 
__U1RXInterrupt:
    PUSH    W0
    
    MOV	    U1RXREG,    W0
    MOV.B   WREG,	U2TXREG
    
    BCLR    IFS0,	#U1RXIF
    
    POP	    W0
    RETFIE
    
    
    
    