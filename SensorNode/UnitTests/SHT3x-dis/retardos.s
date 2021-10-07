.include "p30F4013.inc"
.global _retardo_1S     
.global _retardo_15ms
.global _retardo_20ms
.global _retardo_100ms
.global	_retardo_5ms
    
;******************************************************************************    
; @brief: ESTA RUTINA GENERA UN RETARDO DE 5MS
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************  
_retardo_5ms:
    PUSH    W0
		
    MOV	    #3276,  W0
    CICLO1_5ms:
    DEC	    W0,	    W0
    BRA	    NZ,	    CICLO1_5ms

    POP	    W0
    RETURN

;******************************************************************************    
; @brief: ESTA RUTINA GENERA UN RETARDO DE 15MS
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************      
_retardo_15ms:
    PUSH    W0
    MOV	    #9216,  W0
  
    CICLO_15mS:
    DEC	    W0,	    W0
    BRA	    NZ,	    CICLO_15mS
    
    POP	    W0
    RETURN

;******************************************************************************    
; @brief: ESTA RUTINA GENERA UN RETARDO DE 20MS
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************     
_retardo_20ms:
    CALL    _retardo_5ms
    CALL    _retardo_15ms
    ;CALL    _retardo_5ms
    ;CALL    _retardo_5ms
    
    RETFIE

;******************************************************************************    
; @brief: ESTA RUTINA GENERA UN RETARDO DE 100MS
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************  
_retardo_100ms:
    CALL    _retardo_20ms
    CALL    _retardo_20ms
    CALL    _retardo_20ms
    CALL    _retardo_20ms
    CALL    _retardo_20ms
    ;CALL    _retardo_15ms
    ;CALL    _retardo_15ms
    
    RETFIE
    
;******************************************************************************    
; @brief: ESTA RUTINA GENERA UN RETARDO DE 1S
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************   
_retardo_1S:
    PUSH    W0
    PUSH    W1
    MOV	    #10,    W1
    
    CICLO2_1S:
    CLR	    W0

    CICLO_1S:
    DEC	    W0,	    W0
    BRA	    NZ,	    CICLO_1S
    DEC	    W1,	    W1
    BRA	    NZ,	    CICLO2_1S
   
    POP	    W1
    POP	    W0
    RETURN

    
