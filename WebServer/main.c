/********************************************************************************
 * @brief: Este es el programa principal del servidor web embebido, en el se 
 * recibirán peticiones de los clientes por http y también se recibirán las 
 * tramas de comunicación enviadas por el DSPIC30F4031
 * @device: RASPBERRY PI 3B+
 *******************************************************************************/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "handlers.h"
#include "mongoose.h"

/*Dirección IP y puerto donde se alojará el servidor web*/
#define s_direccion_escucha "http://localhost:8000"

int fin;
/*Directorio raiz donde se alojaran las vistas renderizadas por el servidor*/
const char *dir_raiz = "./public";

void servidor_tcp(void *args);

int main( int argc, char *argv[] ){
    struct mg_mgr mgr;
    struct mg_connection *c;
    pthread_t tid_servidor_tcp;

    pthread_create(&tid_servidor_tcp,NULL,servidor_tcp,NULL);
    signal( SIGINT, manejador_sen );
    mg_mgr_init( &mgr );

    if( ( c = mg_http_listen( &mgr, s_direccion_escucha, manejador_servidor, &mgr) ) == NULL ){
        fprintf( stderr, "Error, no se puede escuchar en la dirección %s", s_direccion_escucha );
        exit( EXIT_FAILURE );
    }
    
    fin = 1;
    while( fin )
        mg_mgr_poll( &mgr, 1000 );
    
    mg_mgr_free( &mgr );

    pthread_join(tid_servidor_tcp,NULL);

    return 0;
}

/******************************************************************
* @brief: Rutina que será ejecutada por un hilo donde se recibirán
* las tramas tcp del microcontrolador
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void servidor_tcp(void *args){
    struct mg_mgr mgr;
    
    LOG(LL_INFO, ("Iniciando Servidor TCP"));
                               
    mg_listen(&mgr, "tcp://127.0.0.1:1234", manejador_tcp, &mgr);
    for (;;) mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);   
    pthread_exit(NULL);
}