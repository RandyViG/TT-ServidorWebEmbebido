.include "p30F4013.inc"
.GLOBAL __U2RXInterrupt

;******************************************************************************    
; @brief: ESTA RUTINA LEE LAS RESPUESTAS DEL MODULO WIFI ENVIADAS A TRAVES
;         DE UART2 Y LAS ENVIA MEDIANTE UART1 HACIA EL MODULO FT232
; @params: NINGUNO
; @return: NINGUNO
;****************************************************************************** 
__U2RXInterrupt:
    PUSH    W0
    
    MOV	    U2RXREG,    W0
    MOV.B   WREG,	U1TXREG
    
    BCLR    IFS1,	#U2RXIF
    
    POP	    W0
    RETFIE
    
    
    
    