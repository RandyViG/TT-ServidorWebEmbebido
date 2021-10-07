#ifndef HANDLERS_H
#define HANDLERS_H

void server_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data);
void signal_handler( int signal );

#endif