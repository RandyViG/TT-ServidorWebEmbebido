    .include "p30F4013.inc"
    .global __U1RXInterrupt

;**@brief: La interrupcion de recepcion del UART1 lee la respuesta del 
; *	   módulo WIFI y la la envia a la PC usando el UART2 donde esta
; *	   conectado el módulo FT232
; *
__U1RXInterrupt:
    PUSH    W0
    
    MOV	    U1RXREG,    W0
    MOV.B   WREG,	U2TXREG
    
    BCLR    IFS0,	#U1RXIF
    
    POP	    W0
    RETFIE
    
    