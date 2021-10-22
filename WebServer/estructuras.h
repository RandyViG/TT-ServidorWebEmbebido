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

struct datos_usuario
{
    char *nombre;
    char *password;
    char *email;
    int nodo;
};