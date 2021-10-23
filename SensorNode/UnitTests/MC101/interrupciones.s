.include "p30F4013.inc"
.GLOBAL __T3Interrupt
.GLOBAL __ADCInterrupt
    
;** @brief: Genera un se?al cuadrada para verificar que 
;*	    la frecuencia de muestro sea correcta
;*  @param: No recibe paramteros
;*  @return: ninguno ( void )
__T3Interrupt:
    BTG		LATD,		#LATD3
    BCLR    	IFS0,		#T3IF
    
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
    LSR	    	W0,		#4,	    W0  ; W0 >> 4
    
    MOV.B   	WREG,		U2TXREG		; U1TXREG = WO(7:0)
    LSR	    	W0,		#8,	    W0	; W0 >> 8
    MOV.B   	WREG,		U2TXREG		; U1TXREG = WO(15:8)
    
    BCLR    	IFS0,		#ADIF
    POP	    	W1
    POP	    	W0
    
    RETFIE
    
    
    

