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
#include "hilo.h"
#include "demonio.h"
#include "handlers.h"
#include "mongoose.h"
#include "estructuras.h"

int fin;
pthread_mutex_t sensores_lock;
/*Directorio raiz donde se alojaran las vistas renderizadas por el servidor*/
const char *dir_raiz = "./public";

int main( int argc, char *argv[] ){
    char direccion[30];
    struct mg_mgr mgr_http,mgr_tcp;
    struct args_thread *args_tcp;
    struct mg_connection *c;
    pthread_t tid_servidor_tcp;

    //iniciar_demonio();

    sprintf(direccion,"https://%s:%d",s_direccion_escucha,s_puerto_escucha);

    if (pthread_mutex_init(&sensores_lock, NULL) != 0){
        printf("Inicialización del mutex para sensores ha fallado!\n");
        return -1;
    }

    args_tcp = malloc(sizeof(struct args_thread));
    args_tcp->mgr = &mgr_tcp;

    signal( SIGINT, manejador_sen );
    mg_mgr_init( &mgr_http );
    //pthread_create(&tid_servidor_tcp,NULL,servidor_tcp,args_tcp);
    pthread_create(&tid_servidor_tcp, NULL, servidor_tcp, (void*)&mgr_tcp);

    if( ( c = mg_http_listen( &mgr_http, direccion, manejador_servidor, &mgr_http) ) == NULL ){
        fprintf( stderr, "Error, no se puede escuchar en la dirección %s", s_direccion_escucha );
        exit( EXIT_FAILURE );
    }
    
    fin = 1;
    while( fin )
        mg_mgr_poll( &mgr_http, 1000 );
    
    LOG(LL_INFO,("TERMINANDO SERVIDOR HTTP"));
    pthread_kill(tid_servidor_tcp, SIGINT);

    mg_mgr_free( &mgr_http );
    mg_mgr_free( &mgr_tcp );
    free(args_tcp);
    pthread_mutex_destroy(&sensores_lock);
    return 0;
}
