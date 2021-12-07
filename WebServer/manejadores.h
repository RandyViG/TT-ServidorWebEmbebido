#include "mongoose.h"
#ifndef MANEJADORES_H
#define MANEJADORES_H

void manejador_servidor(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
void manejador_tcp(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
void manejador_sen( int sen );

#endif