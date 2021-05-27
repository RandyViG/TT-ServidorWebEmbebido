#include<signal.h>
#include"mongoose.h"

extern int end;
extern const char *s_root_dir;

void signal_handler( int signal ){
    if( signal == SIGINT )
        end = 0;
}

void server_handler( struct mg_connection *c, int ev, void *ev_data, void *fn_data ){
    if( ev == MG_EV_HTTP_MSG ){
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    	if( mg_http_match_uri( hm, "/hi" ) ){
            char name[100];
           
            if( mg_http_get_var( &hm->query, "name", name, 100 ) > 0 )
                mg_http_reply( c, 200, "", "Hola %s, desde Mongoose v%s", name, MG_VERSION);
            else
                mg_http_reply( c, 200, "", "Hubo un error con el parametro, desde Mongoose v%s", MG_VERSION);
        }

        else if( mg_http_match_uri( hm, "/login" ) )
            mg_http_serve_file( c, hm, "./public/login.html", "text/html", "\n\r\n" );

        else if( mg_http_match_uri( hm, "/form" ) )
            mg_http_serve_file( c, hm, "./public/form_user.html", "text/html", "\n\r\n" );

        else if( mg_http_match_uri( hm, "/admin" ) )
            mg_http_serve_file( c, hm, "./public/admin.html", "text/html", "\n\r\n" );

        else if( mg_http_match_uri( hm, "/form-admin" ) )
            mg_http_serve_file( c, hm, "./public/data-admin.html", "text/html", "\n\r\n" );

        else if( mg_http_match_uri( hm, "/alert" ) )
            mg_http_serve_file( c, hm, "./public/indexAlert.html", "text/html", "\n\r\n" );

        else{
            struct mg_http_serve_opts opts = { .root_dir = s_root_dir };
    	    mg_http_serve_dir(c, ev_data, &opts);
        }
            
    }

    return (void) fn_data;
}