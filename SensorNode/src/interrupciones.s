.include "p30F4013.inc"
.global __U2RXInterrupt
.global __T3Interrupt
.global __ADCInterrupt

.global _bandera 
.global _gasLP
    

;******************************************************************************    
; @brief: ESTA RUTINA LEE LAS RESPUESTAS DEL MODULO WIFI ENVIADAS A TRAVES
;         DE UART1 Y LAS ENVIA MEDIANTE UART2 HACIA EL MODULO FT232
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
    

;** @brief: Genera un señal cuadrada para verificar que 
;*	    la frecuencia de muestro sea correcta
;*  @param: No recibe paramteros
;*  @return: ninguno ( void )
__T3Interrupt:
    PUSH	W0
    
    BTG		LATD,		#LATD3
    MOV		#1,		W0
    MOV		W0,		_bandera
    BCLR    	IFS0,		#T3IF
    NOP
    
    POP		W0
    
    RETFIE

;** @brief: Cuando el ADC tiene 16 muestras se activa la ISR
;*	    aplica un filtro digital (FIR) y se manda al FIFO
;*	    del UART1 para su transmisi?n.
;*  @param: No recibe paramteros
;*  @return: ninguno ( void )
__ADCInterrupt:
    PUSH    	W0
    PUSH    	W1
    CLR	    	W0
    MOV	    	#ADCBUF0,	W1		; W1 = &ADCBUF0
    
    REPEAT  	#15
    ADD	    	W0,		[W1++],	    W0  ; W0 = W0 + [W1++]
    LSR	    	W0,		#4,	    W0  ; W0 >> 4 | W0 = W0 / 4
    
    MOV		W0,		_gasLP
    
    BCLR    	IFS0,		#ADIF
    POP	    	W1
    POP	    	W0
    
    RETFIE
    
    
