.include "p30F4013.inc"

.GLOBAL	_iniciar_I2C
.GLOBAL	_enviar_dato_I2C
.GLOBAL	_recibe_dato_I2C
.GLOBAL	_ack_I2C
.GLOBAL _nack_I2C
.GLOBAL	_detener_I2C
.GLOBAL	_reiniciar_I2C
    
;******************************************************************************    
; @brief: ESTA RUTINA GENERA LA CONDICIÓN DE INICIO (START) AL BUS I2C Y 
; VERIFICA QUE EL DISPOSITIVO ESCLAVO MANDE UN BIT DE RECONOCIMIENTO EN EL
; NOVENO CICLO DEL RELOJ
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************    
;******************************************************************************
; DESCRIPCION: ESTA RUTINA GENERA LA CONDICION DE START AL BUS I2C, SE 
; RECOMIENDA VERIFICAR EL BIT P DE I2CSTAT ANTES DE LLAMAR A 
; ESTA FUNCION, PARA SABER SI EL BUS ESTA LIBRE. 
; SI P = 1, ENTONCES EL BUS ESTA LIBRE
; PARAMETROS: NINGUNO
; RETORNO: NINGUNO
;******************************************************************************
_iniciar_I2C:
	BCLR		IFS0,			#MI2CIF
	BSET		I2CCON,			#SEN
ESPERA_START:
	BTSS		IFS0,			#MI2CIF
	GOTO		ESPERA_START

	RETURN

;******************************************************************************    
; @brief: ESTA RUTINA MANDA UN DATO DE 8 BITS AL DISPOSITIVO ESCLAVO Y 
; VERIFICA QUE EL DISPOSITIVO ESCLAVO MANDE UN BIT DE RECONOCIMIENTO EN EL
; NOVENO CICLO DEL RELOJ
; @params: W0 (WREG) TIENE EL DATO QUE SE ENVIARA AL SENSOR
; @return: NINGUNO
;******************************************************************************  
_enviar_dato_I2C:
	BCLR		IFS0,			#MI2CIF
	MOV.B		WREG,			I2CTRN
ESPERA_envia_dato_I2C:
	BTSS		IFS0,			#MI2CIF
	GOTO		ESPERA_envia_dato_I2C
	
	RETURN

;******************************************************************************    
; @brief: ESTA RUTINA RECIBE UN DATO DE 8 BITS DE UN DISPOSITIVO ESCLAVO
; @params: NINGUNO
; @return: W0 TIENE EL DATO RECUPERADO DEL SENSOR
;******************************************************************************  
_recibe_dato_I2C:
	BCLR		IFS0,			#MI2CIF
	BSET		I2CCON,			#RCEN

ESPERA_recibe_dato_I2C:
	BTSS		IFS0,			#MI2CIF
	GOTO		ESPERA_recibe_dato_I2C

	MOV		I2CRCV,			W0
	RETURN

;******************************************************************************    
; @brief: ESTA RUTINA GENERA LA CONDICION ACK DEL MAESTRO HACIA EL ESCLAVO
;         PARA CONFIRMAR LA RECEPCIÓN DE UN BYTE
;         LA CONDICIÓN SE DEFINE EN EL REGISTRO I2CCON EN EL BIT ACKDT:
;           1, nack
;           0, ack
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************  
_ack_I2C:
	BCLR		IFS0,			#MI2CIF
	BCLR		I2CCON,			#ACKDT
	BSET		I2CCON,			#ACKEN
	
ESPERA_ack_I2C:
	BTSS		IFS0,			#MI2CIF
	GOTO		ESPERA_ack_I2C
	
	RETURN

;******************************************************************************    
; @brief: ESTA RUTINA GENERA LA CONDICION NACK DEL MAESTRO HACIA EL ESCLAVO
;         PARA CONFIRMAR LA RECEPCIÓN DE UN BYTE
;         LA CONDICIÓN SE DEFINE EN EL REGISTRO I2CCON EN EL BIT ACKDT:
;           1, nack
;           0, ack
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************  
_nack_I2C:
	BCLR		IFS0,			#MI2CIF
	BSET		I2CCON,			#ACKDT
	BSET		I2CCON,			#ACKEN
ESPERA_nack_I2C:
	BTSS		IFS0,			#MI2CIF
	GOTO		ESPERA_nack_I2C
	
	RETURN

;******************************************************************************    
; @brief: ESTA RUTINA GENERA LA CONDICIÓN DE PARAR (STOP) AL BUS DE I2C Y 
;         VERIFICA QUE EL DISPOSITIVO ESCLAVO MANDE UN BIT DE RECONOCIMIENTO 
;         EN EL NOVENO CICLO DEL RELOJ
; @params: NINGUNO
; @return: NINGUNO
;******************************************************************************  
_detener_I2C:
	BCLR		IFS0,			#MI2CIF
	BSET		I2CCON,			#PEN
ESPERA_detener_I2C:
	BTSS		IFS0,			#MI2CIF
	GOTO		ESPERA_detener_I2C
	
	RETURN

;******************************************************************************    
; @brief: ESTA RUTINA GENERA LA CONDICIÓN DE REINICIAR (RESTART) AL BUS DE 
;         I2C Y VERIFICA QUE EL DISPOSITIVO ESCLAVO MANDE UN BIT DE 
;         RECONOCIMIENTO EN EL NOVENO CICLO DEL RELOJ
; @params: NINGUNO
; @return: NINGUNO
;****************************************************************************** 
_reiniciar_I2C:
	BCLR		IFS0,			#MI2CIF
	BSET		I2CCON,			#RSEN
ESPERA_reiniciar_I2C:
	BTSS		IFS0,			#MI2CIF
	GOTO		ESPERA_reiniciar_I2C
	
	RETURN



