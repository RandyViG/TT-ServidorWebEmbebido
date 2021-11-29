#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "handlers.h"
#include "mongoose.h"
#include "estructuras.h"

/******************************************************************
* @brief: Rutina que será ejecutada por un hilo donde se recibirán
* las tramas tcp del microcontrolador
* @param: NINGUNO                                                      
* @return: NINGUNO														
******************************************************************/
void * servidor_tcp(void *args){
    struct mg_mgr *mgr;
    char direccion[40];
    //mgr = ((struct args_thread *)args)->mgr;
    mgr = (struct mg_mgr *)args;
    sprintf(direccion,"tcp://%s:%d",sock_direccion_escucha,sock_puerto_escucha);   
    LOG(LL_INFO, ("TCP DIRECCION: %s",direccion));
    LOG(LL_INFO, ("Iniciando Servidor TCP"));
    mg_mgr_init( mgr );

    mg_listen(mgr, direccion, manejador_tcp, mgr);

    for (;;) mg_mgr_poll(mgr, 1000);
    
    LOG(LL_INFO,("Terminando Servidor TCP"));
    
    pthread_exit(NULL);
}

