#include "mongoose.h"
#include "estructuras.h"
#include "mjson.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int generar_puerto(){
    int puerto;
    srand(time(NULL)*clock());
    puerto = (rand() % (30000 - 3000 + 1)) + 3000;
    return puerto;
}

void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    mg_ws_upgrade(c, hm, NULL);  // Upgrade HTTP to WS
  }
  if (ev == MG_EV_WS_OPEN) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    mg_ws_send(c, "opened", 6, WEBSOCKET_OP_BINARY);  // Send "opened" to web socket connection
  }
}

int crear_ws(int puerto){
    struct mg_mgr mgr;
    char direccion[30];

    sprintf(direccion,"%s:%d",s_direccion_escucha,puerto);              
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, direccion, fn, NULL);     // Create listening connection
    for (;;) mg_mgr_poll(&mgr, 1000);              // Block forever
}