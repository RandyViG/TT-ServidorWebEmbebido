#include "mongoose.h"

/*Dirección IP y puerto donde se alojará el servidor web*/
#define s_direccion_escucha "http://localhost"
#define s_puerto_escucha 8000

/******************************************************************
* @brief: Estructura usada para generar los archivos con los datos
* de medición de los sensores.
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
struct datos_recibidos{
    float medicion_gas;
    float medicion_temp;
    float medicion_hum;
    int bandera_alerta;
};


/******************************************************************
* @brief: Estructura usada para guardar los datos del usuario.
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
struct datos_usuario{
    char *nombre;
    char *password;
    char *email;
    int nodo;
};

/******************************************************************
* @brief: Estructura usada para guardar los datos de sesión.
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
struct datos_sesion{
    int id;
    uint64_t sha;
    char *usuario;
    time_t creacion;
};

struct lista_ws{
    int puerto;
    pthread_t tid_ws;
    struct lista_ws *sig;
};