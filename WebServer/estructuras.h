#include "mongoose.h"

/*Dirección IP y puerto donde se alojará el servidor web*/
#define s_direccion_escucha "192.168.15.12"
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
    time_t alerta;
};


/******************************************************************
* @brief: Estructura usada para guardar los datos del usuario.
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
struct datos_usuario
{
    char *nombre;
    char *password;
    char *email;
    short int nodo;
    short int admin;
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

struct args_ws_thread{
    struct mg_mgr mgr;
    int puerto;
};