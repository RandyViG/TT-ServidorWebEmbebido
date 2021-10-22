#include <signal.h>
#include "mongoose.h"
#include "procesamiento.h"

/*Variables globales*/
extern int fin;
extern const char *dir_raiz;

/******************************************************************
* @brief: Manejador encargado de procesar las señales mandadas al
* servidor
* @param: señal                                                      
* @return: NINGUNO														
******************************************************************/
void manejador_sen( int sen ){
    if( sen == SIGINT )
        fin = 0;
}

/******************************************************************
* @brief: Manejador encargado de procesar los eventos de peticiones
* hacia el servidor web
* @param: Estructura de conexión, evento, datos de evento y datos 
* a para el manejador                                                      
* @return: Datos de función													
******************************************************************/
void manejador_servidor( struct mg_connection *c, int ev, void *datos_ev, void *datos_fn ){
    if( ev == MG_EV_HTTP_MSG ){
        struct mg_http_message *hm = (struct mg_http_message *) datos_ev;
        struct mg_http_serve_opts opts = {.mime_types = "text/html",.extra_headers = "AA: bb\r\nCC: dd\r\n"};
    	
        if( mg_http_match_uri( hm, "/hi" ) ){
            char name[100];
           
            if( mg_http_get_var( &hm->query, "name", name, 100 ) > 0 )
                mg_http_reply( c, 200, "", "Hola %s, desde Mongoose v%s", name, MG_VERSION);
            else
                mg_http_reply( c, 200, "", "Hubo un error con el parametro, desde Mongoose v%s", MG_VERSION);
        }

        /*Definición de rutas*/
        else if(mg_http_match_uri( hm, "/" )){
            mg_http_serve_file( c, hm, "./public/login.html",&opts);
        }
        else if( mg_http_match_uri( hm, "/login" ) )
            mg_http_serve_file( c, hm, "./public/login.html",&opts);

        else if( mg_http_match_uri( hm, "/form" ) )
            mg_http_serve_file( c, hm, "./public/form_user.html",&opts);

        else if( mg_http_match_uri( hm, "/admin" ) )
            mg_http_serve_file( c, hm, "./public/admin.html",&opts);

        else if( mg_http_match_uri( hm, "/form-admin" ) )
            mg_http_serve_file( c, hm, "./public/data-admin.html",&opts);

        else if( mg_http_match_uri( hm, "/alert" ) )
            mg_http_serve_file( c, hm, "./public/indexAlert.html",&opts);

        else{
            struct mg_http_serve_opts opts2 = { .root_dir = dir_raiz };
    	    mg_http_serve_dir(c, datos_ev, &opts2);
        }
            
    }
}

/******************************************************************
* @brief: Manejador encargado de procesar los eventos de recepción
* de datos enviados desde el microcontrolador
* @param: Estructura de conexión, evento, datos de evento y datos 
* a para el manejador                                                      
* @return: NINGUNO												
******************************************************************/

void manejador_tcp(struct mg_connection *c, int ev, void *datos_ev, void *datos_fn) {
    if (ev == MG_EV_READ) {
        struct datos_recibidos dr;
        LOG(LL_INFO, ("Algo ha sido recibido"));

        procesar_cadena((unsigned char*)(c->recv.buf),&dr);

        mg_printf(c, "Correcto!");
        mg_iobuf_del(&c->recv, 0, c->recv.len);   // And discard it
    }
}