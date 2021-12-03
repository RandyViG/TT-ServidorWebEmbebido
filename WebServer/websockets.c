#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "mongoose.h"
#include "estructuras.h"
#include "procesamiento.h"

int generar_puerto(){
    int puerto;
    srand(time(NULL)*clock());
    puerto = (rand() % (10000 - 8001 + 1)) + 8001;
    return puerto;
}

void *responder_client(void *args){
  struct args_ws_client *args_s;
  struct datos_recibidos datos;
  char strS[300];
  int len;
  args_s = ((struct args_ws_client *)args);
  // LOG(LL_INFO,("ENTRO HILO %d",args_s->nodo));

  leer_medidas(args_s->nodo,&datos);
  sprintf(strS,"{\"hum\":%.2f,\"gas\":%.2f,\"temp\":%.2f}",datos.medicion_hum,datos.medicion_gas,datos.medicion_temp);
  len=str_len(strS);
  sleep(1);
  mg_ws_send(args_s->c,strS, len-1, WEBSOCKET_OP_TEXT);
  // LOG(LL_INFO,("SALGO HILO"));

  free(args_s->ptr);
  pthread_exit(NULL);
}

void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  // if (ev == MG_EV_ACCEPT) {
  //   struct mg_tls_opts opts = {
  //     .cert = "cert.pem",    // Certificate file
  //     .certkey = "key.pem",  // Private key file
  //   };
  //   mg_tls_init(c, &opts);
  // }
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    mg_ws_upgrade(c, hm, NULL);  // Upgrade HTTP to WS
  }
  if (ev == MG_EV_WS_OPEN) {
    mg_ws_send(c, "{}", 2, WEBSOCKET_OP_TEXT);  // Send "opened" to web socket connection
  }
  if (ev == MG_EV_WS_MSG) {
    struct mg_ws_message *msg;
    msg = (struct mg_ws_message *)ev_data;
    // LOG(LL_INFO,("MSG: %s",msg->data.ptr));
    if(!strcmp("SMO",msg->data.ptr)){
        pthread_t tid;
        struct args_ws_client *args;

        args = (struct args_ws_client *) malloc(sizeof(struct args_ws_client));
        args->c = c;
        args->nodo = *(int*)fn_data;
        args->ptr = args;
        pthread_create(&tid,NULL,responder_client,args);
    }
  }
  if(ev == 12){
    //LOG(LL_INFO,("Borrar hilo %d",getpid()));
    //pthread_exit(NULL);
  }
}

void *lanzar_servidor_ws(void *args){
    char direccion[30];
    struct mg_mgr mgr = ((struct args_ws_thread*)args)->mgr;
    int puerto = ((struct args_ws_thread*)args)->puerto;
    int nodo = ((struct args_ws_thread*)args)->nodo;

    // printf("\nH:%d\n",puerto);

    sprintf(direccion,"http://%s:%d",s_direccion_escucha,puerto);      
    // sprintf(direccion,"%s:%d","http://localhost",puerto);
    mg_http_listen(&mgr, direccion, fn,&nodo);
    for (;;) mg_mgr_poll(&mgr, 1000);

    pthread_exit(NULL);
}

void *iniciar_ws(void *args){
    pthread_t tid;
    // int puerto = *((int*)args);//8001;//generar_puerto();
    struct mg_mgr mgr;
    struct args_ws_thread *aux_args = ((struct args_ws_thread*)args);
    struct args_ws_thread s_args;

    s_args.puerto = aux_args->puerto;
    s_args.nodo = aux_args->nodo;

    mg_mgr_init(&mgr);
    s_args.mgr = mgr;

    pthread_create(&tid,NULL,lanzar_servidor_ws,&s_args);

    pthread_join(tid,NULL);
    LOG(LL_INFO,("Limpiando WS!"));
    
    mg_mgr_free(&mgr);
    free(aux_args);
    pthread_exit(NULL);
}


//Crear ws
void crear_ws(int puerto, int nodo){
    pthread_t tid;
    //Aquí
    // int *ptr = (int*) malloc(sizeof(int));
    // *ptr = puerto;
    struct args_ws_thread *args = (struct args_ws_thread *) malloc(sizeof(struct args_ws_thread));
    args->puerto = puerto;
    args->nodo = nodo;
    pthread_create(&tid,NULL,iniciar_ws,args);
    // pthread_join(tid,NULL);
    // printf("Terminando WS!");
}