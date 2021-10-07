#include<stdlib.h>
#include<signal.h>
#include"handlers.h"
#include"mongoose.h"

#define s_listening_address "http://localhost:8000"

int end;
const char *s_root_dir = "./public";

int main( int argc, char *argv[] ){
    struct mg_mgr mgr;
    struct mg_connection *c;

    signal( SIGINT, signal_handler );
    mg_mgr_init( &mgr );

    if( ( c = mg_http_listen( &mgr, s_listening_address, server_handler, &mgr) ) == NULL ){
        fprintf( stderr, "Error, no se puede escuchar en la dirección %s", s_listening_address );
        exit( EXIT_FAILURE );
    }
    
    end = 1;
    while( end )
        mg_mgr_poll( &mgr, 1000 );
    
    mg_mgr_free( &mgr );

    return 0;
}
