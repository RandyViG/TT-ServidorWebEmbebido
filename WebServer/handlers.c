#include <signal.h>

#include "estructuras.h"
#include "mongoose.h"
#include "mjson.h"
#include "procesamiento.h"
#include "sesion.h"
#include "usuario.h"
#include "websockets.h"

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
    if (ev == MG_EV_ACCEPT) {
    struct mg_tls_opts opts = {
      .cert = "cert.pem",    // Certificate file
      .certkey = "key.pem",  // Private key file
    };
    mg_tls_init(c, &opts);
    }
    if( ev == MG_EV_HTTP_MSG ){
        struct mg_http_message *hm = (struct mg_http_message *) datos_ev;
        struct mg_http_serve_opts opts = {.mime_types = "text/html",.extra_headers = "Access-Control-Allow-Origin: *\r\n"};
        // LOG(LL_INFO,("Algo se ha recibido en el servidor HTTP"));                      

        if( mg_http_match_uri( hm, "/hi" ) ){
            char name[100];
           
            if( mg_http_get_var( &hm->query, "name", name, 100 ) > 0 )
                mg_http_reply( c, 200, "", "Hola %s, desde Mongoose v%s", name, MG_VERSION);
            else
                mg_http_reply( c, 200, "", "Hubo un error con el parametro, desde Mongoose v%s", MG_VERSION);
        }
        /*Definición de Endpoints*/
        else if(mg_http_match_uri( hm, "/" )){        
            /*Se obtiene la cookie desde la cabecera de la petición del cliente.*/        
            /*Si la petición no contiene cookie o los datos son inválidos el cliente es dirigido al login
              casi contrario se le concede acceso a los datos del sensor.*/        
            struct mg_str *s = mg_http_get_header(hm, "Cookie");

            if(s != NULL){
                char head_buff[s->len + 2];
                strncpy(head_buff,s->ptr,s->len);
                // LOG(LL_INFO,("COOK_ %s LEN_ %ld",head_buff,s->len));
                if(validar_cookie(head_buff,s->len) > 0){
                    mg_http_serve_file( c, hm, "./public/index.html",&opts);

                }else{
                    mg_http_serve_file( c, hm, "./public/login.html",&opts);
                }
            }else{
                mg_http_serve_file( c, hm, "./public/login.html",&opts);
            }       
        }
        else if( mg_http_match_uri( hm, "/login" ) ){
            mg_http_serve_file( c, hm, "./public/login.html",&opts);
        }
        else if(mg_http_match_uri( hm, "/login_data" )){
            /*Ruta que valida si ls datos de sesión mandados por el usuario son válidos o no.*/
            /*Si los datos son válidos son correctos se regresan los datos de sesión al usuario
            y se registran en el repositorio de datos.*/
            int n;
            char usr[100],psw[100],str_sha[22];
            struct datos_sesion sesion;
            n = mjson_get_string(hm->body.ptr,hm->body.len,"$.usr",usr,sizeof(usr));
            if(n>0){
                n = mjson_get_string(hm->body.ptr,hm->body.len,"$.psw",psw,sizeof(psw));
                if(n>0){
                    if(validar_usuario(usr,psw)>0){
                        if(agregar_sesion(usr,&sesion) > 0){
                            #if defined(__arm__)
                            sprintf(str_sha,"%llu",sesion.sha);
                            #else
                            sprintf(str_sha,"%lu",sesion.sha);
                            #endif
                            mg_http_reply(c, 200, "Content-Type: application/json\r\n"
                            "Access-Control-Allow-Origin: *\r\n","{\"result\": %d,\"id\": %d,\"sha\": \"%s\",\"user\": \"%s\"}", 200,sesion.id,str_sha,sesion.usuario);
                            #if defined(__arm__)
                            LOG(LL_INFO,("sha_ %llu ",sesion.sha));
                            #else
                            LOG(LL_INFO,("sha_ %lu ",sesion.sha));
                            #endif
                        }else{
                            LOG(LL_ERROR,("NO SE HA PODIDO AGREGAR SESION!"));
                        }
                    }else{
                        mg_http_reply(c, 403, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}", 404);
                    }
                }else{
                    LOG(LL_ERROR,("MAL PETICIÓN PSW"));
                        mg_http_reply(c, 400, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}", 400);

                }
            }
            else{
                LOG(LL_ERROR,("MAL PETICIÓN USR!"));
                mg_http_reply(c, 400, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}", 400);
                LOG(LL_ERROR,("MSG: %s",hm->body.ptr));
            }

        }
        else if(mg_http_match_uri( hm, "/configuracion" )){
            struct mg_str *s = mg_http_get_header(hm, "Cookie");
            struct datos_sesion sesion;
            int id,admin,len;

            if(s != NULL){
                char head_buff[s->len + 2];
                strncpy(head_buff,s->ptr,s->len);

                id = buscar_id_sesion(head_buff,s->len);
                if(buscar_sesion_por_id(id,&sesion) > 0){
                    len = str_len(sesion.usuario);
                    admin = verificar_administrador(sesion.usuario, len);
                    // printf("ID: %d, NOM: %s ,ADM: %d",id,sesion.usuario,admin);
                    if(admin == 1){
                        mg_http_serve_file( c, hm, "./public/admin.html",&opts);
                    }else if(admin == 0){
                        mg_http_serve_file( c, hm, "./public/form_user.html",&opts);
                    }else{
                        LOG(LL_ERROR,("Sesión no válida"));
                    }
                }else{
                    LOG(LL_ERROR,("Sesión no encontrada"));
                }
            }
        }
        else if( mg_http_match_uri( hm, "/logout" ) ){
            /*Se eliminan los datos de sesión del cliente*/
            struct mg_str *s = mg_http_get_header(hm, "Cookie");
            int id;
            if(s != NULL){
                char head_buff[s->len + 2];
                strncpy(head_buff,s->ptr,s->len);

                id = buscar_id_sesion(head_buff,s->len);
                if (eliminar_sesion(id)>0){
                    LOG(LL_INFO,("Sesión eliminada correctamente"));
                    mg_http_serve_file( c, hm, "./public/login.html",&opts);
                }else{
                    LOG(LL_ERROR,("Error al eliminar sesión"));
                    mg_http_reply(c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}", 500);
                }
            }
        }
        else if(mg_http_match_uri( hm, "/datos_sensor" )){
            // Crear el ws
            int puerto = generar_puerto();
            //printf("S: %d",puerto);
            mg_http_reply( c, 200, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"port\":%d,\"result\": %d}", puerto,200);
            crear_ws(puerto);
        }
        else if( mg_http_match_uri( hm, "/datos_usuarios" ) ){
            size_t file_size;
            char* data = mg_file_read("usuarios.json", &file_size);
            if (data != NULL) {
                mg_http_reply(c, 200, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "%s",data);
            }else{
                mg_http_reply( c, 400, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",400);
            }
            free(data);
        }
        else if( mg_http_match_uri( hm, "/edit_usuario" ) ){
            struct datos_sesion sesion;
            double session;
            int id,len,admin,n1,n2,n3,e;
            char nUsr[50],nPsw[50],email[60];
            mjson_get_number(hm->body.ptr,hm->body.len,"$.session",&session);
            id=(int)session;

            if(buscar_sesion_por_id(id,&sesion) > 0){
                    len = str_len(sesion.usuario);
                    admin = verificar_administrador(sesion.usuario, len);
                    // LOG(LL_INFO,("ID: %d, NOM: %s ,ADM: %d",id,sesion.usuario,admin));
                    if(admin == 1){
                        //HACER PARA ADMINISTRADOR
                        int n4,n5,nodo;
                        double aux_nodo;
                        char target[50];

                        n1 = mjson_get_string(hm->body.ptr,hm->body.len,"$.usr",nUsr,sizeof(nUsr));
                        n2 = mjson_get_string(hm->body.ptr,hm->body.len,"$.psw",nPsw,sizeof(nPsw));
                        n3 = mjson_get_string(hm->body.ptr,hm->body.len,"$.target",target,sizeof(target));
                        n4 = mjson_get_string(hm->body.ptr,hm->body.len,"$.email",email,sizeof(email));
                        n5 = mjson_get_number(hm->body.ptr,hm->body.len,"$.nodo",&aux_nodo);

                        nodo = (int)aux_nodo;
                        LOG(LL_INFO,("CAMBIAR %s A %s CON PSW %s EMAIL %s Y EN NODO %d",target,nUsr,nPsw,email,nodo));
                        if(n1 >= 0 && n2 > 0 && n3 > 0 && n4 >= 0 && n5 > 0){
                            e = actualizar_usuario_admin(target,nUsr,nPsw,email,nodo);
                            if(e>0){
                                mg_http_reply( c, 200, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",200);
                            }else{
                                LOG(LL_ERROR,("Error al actualizar usuario"));
                                mg_http_reply( c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",500);
                            }
                        }

                        mg_http_reply( c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",500);

                    }else if(admin == 0){
                        //HACER PARA NO ADMINISTRADOR
                        n1 = mjson_get_string(hm->body.ptr,hm->body.len,"$.usr",nUsr,sizeof(nUsr));
                        n2 = mjson_get_string(hm->body.ptr,hm->body.len,"$.psw",nPsw,sizeof(nPsw));
                        n3 = mjson_get_string(hm->body.ptr,hm->body.len,"$.email",nPsw,sizeof(nPsw));
                        if(n1 > 0 && n2 > 0 && n3 > 0){
                            e = actualizar_usuario_admin(sesion.usuario,nUsr,nPsw,email,0);
                            if(e>0){
                                mg_http_reply( c, 200, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",200);
                            }else{
                                LOG(LL_ERROR,("Error al actualizar usuario"));
                                mg_http_reply( c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",500);
                            }
                        }
                    }else{
                        LOG(LL_ERROR,("Sesión no válida"));
                        mg_http_reply( c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",500);

                    }
                }else{
                    LOG(LL_ERROR,("Sesión no encontrada"));
                    mg_http_reply( c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",500);
                }           
        }
        else if( mg_http_match_uri( hm, "/eliminar_usuario" ) ){
            char user[50];
            if( mg_http_get_var( &hm->query, "user", user, 100 ) > 0 ){
                user[hm->query.len + 1]='\0';
                printf("Eliminar usuario %s",user);
                if(eliminar_usuario(user) > 0){
                    LOG(LL_INFO,("Usuario %s eliminado!",user));
                    mg_http_serve_file( c, hm, "./public/index.html",&opts);
                }else{
                    LOG(LL_INFO,("Error al eliminar al usuario %s!",user));

                }
            }                
        }
        else if( mg_http_match_uri( hm, "/agregar_usuario" ) ){
            int n1,n2,n3,n4,e;
            double aux_nodo;
            char nUsr[50],nPsw[50],email[60];

            n1 = mjson_get_string(hm->body.ptr,hm->body.len,"$.usr",nUsr,sizeof(nUsr));
            n2 = mjson_get_string(hm->body.ptr,hm->body.len,"$.psw",nPsw,sizeof(nPsw));
            n4 = mjson_get_string(hm->body.ptr,hm->body.len,"$.email",email,sizeof(email));
            n3 = mjson_get_number(hm->body.ptr,hm->body.len,"$.nodo",&aux_nodo);

            if(n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0){
                LOG(LL_INFO,("NUEVO USR %s CON PSW %s EMAIL %s NODO %d",nUsr,nPsw,email,(int)aux_nodo));
                //nodo = (int)aux_nodo;
                e = agregar_usuario(nUsr, nPsw, email, (int)aux_nodo, 0);
                if(e>0){                
                    mg_http_reply( c, 200, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",200);
                }else{
                    LOG(LL_ERROR,("Error al agregar usuario"));
                    mg_http_reply( c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",500);            
                }
            }else{
                mg_http_reply( c, 500, "Content-Type: application/json\r\n""Access-Control-Allow-Origin: *\r\n", "{\"result\": %d}",500);            
            }
        }
        else if( mg_http_match_uri( hm, "/form_admin" ) ){
            mg_http_serve_file( c, hm, "./public/data-admin.html",&opts);
        }
        else if( mg_http_match_uri( hm, "/admin" ) ){
            mg_http_serve_file( c, hm, "./public/admin.html",&opts);
        }
        else if( mg_http_match_uri( hm, "/form" ) )
            mg_http_serve_file( c, hm, "./public/form-user.html",&opts);

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
        LOG(LL_INFO, ("Una trama ha sido recibida"));

        procesar_cadena((unsigned char*)(c->recv.buf),&dr);

        mg_printf(c, "Correcto!");
        mg_iobuf_del(&c->recv, 0, c->recv.len);
    }
}