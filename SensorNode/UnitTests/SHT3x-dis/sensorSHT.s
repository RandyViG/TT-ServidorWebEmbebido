.include "p30F4013.inc"
.global _reiniciar_SHT
    
.equ	RST,	RD0

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


