#include <stdio.h>
#include <stdlib.h>

#include "estructuras.h"
#include "procesamiento.h"
#include "mjson.h"
#include "mongoose.h"

/******************************************************************
* @brief: Agrega una sesión nueva en el repositorio de sesiones json
* que se relacionará a un usuario
* @param: Nombre del usuario que inicio sesión y estructura donde
* se llenarán los datos.
* @return: 1 si la estructura se llenó correctamente, -1 si no.												
******************************************************************/
int agregar_sesion(char *usuario,struct datos_sesion *sesion){

    FILE *fpr;
    char c;

    int id,len_usr,i=0,flag=0,max_len;
    time_t creado_el = time(0);
    char id_str[10],str_put[500];
    mg_sha1_ctx ctx;
    unsigned char dig[20];

    srand(time(NULL)*clock());
    id = rand();

    sprintf(id_str,"%d",id);
    mg_sha1_init(&ctx);
    mg_sha1_update(&ctx,(unsigned char *)id_str,10);
    mg_sha1_update(&ctx,(unsigned char *)"CHSMPP",7);
    mg_sha1_final(dig,&ctx);

    // max_len = 84 + 10 + 18 + str_len(usuario) + 10;

    fpr=fopen("sesiones.json","r+");

    if(fpr==NULL){
        printf("Hubo un problema al abrir el archivo");
        return -1;
    }
    else{
        while(1){
            fseek(fpr,-i,SEEK_END);
            c = fgetc(fpr);
            //printf("%c",c);
            i+=1;
            if((int)c == -1){
                if(flag > 0){
                    fprintf(fpr,"[\n\t{\n\t\"id\":\"%d\",\n\t\"sha\":\"%llu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n]",id,*(uint64_t *) dig,usuario,creado_el);
                    break;
                }
                flag++;
            }
            if(c == '['){        
                // "\n\t{\n\t\"id\":\"%d\",\n\t\"sha\":\"%llu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n]",id,*(uint64_t *) dig,usuario,creado_el
                sprintf(str_put,"\n\t{\n\t\"id\":\"%d\",\n\t\"sha\":\"%llu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}",id,*(uint64_t *) dig,usuario,creado_el);
                max_len = str_len(str_put) + 1;
                LOG(LL_INFO,("Tamaño de i: %d, len: %d",i,max_len));
                if(i-1<=max_len)
                    fprintf(fpr,"%s\n]",str_put);
                else
                    fprintf(fpr,"%s\n",str_put);
                break;
            }    
            if(c == '}'){
                sprintf(str_put,"\n\t,{\n\t\"id\":\"%d\",\n\t\"sha\":\"%llu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}",id,*(uint64_t *) dig,usuario,creado_el);
                max_len = str_len(str_put) + 1;
                LOG(LL_INFO,("Tamaño de i: %d, len: %d",i,max_len));
                
                if(i-1<=max_len)                
                    fprintf(fpr,"%s\n]",str_put);
                else
                    fprintf(fpr,"%s\n",str_put);
                break;
            }
        }        
    }

    sesion->id = id;
    sesion->sha = *(uint64_t *) dig;
    sesion->creacion = creado_el;

    len_usr = strlen(usuario);
    sesion->usuario = (char*) malloc(len_usr*sizeof(char));
    strcpy(sesion->usuario,usuario);
    fclose(fpr);
    return 1;
}
/******************************************************************
* @brief: Llena una estructura en c a partir de una cadena en 
* formato JSON.
* @param: Estructura donde se llenarán los datos, cadena JSON 
* y longitud de la cadena.
* @return: 1 si la estructura se llenó correctamente, -1 si no.												
******************************************************************/
int llenar_struct_sesion(struct datos_sesion *sesion,char *json, int len){
    char buff[100];
    int n;

    n = mjson_get_string(json,len,"$.id",buff,sizeof(buff));
    if(n > 0){
        int x = atoi(buff);
        sesion->id = x;
    }else{
        return -1;
    }

    n = mjson_get_string(json,len,"$.sha",buff,sizeof(buff));
    if(n > 0){
        sesion->sha = atol(buff);
    }else{
        return -1;
    }

    n = mjson_get_string(json,len,"$.usuario",buff,sizeof(buff));
    if(n > 0){
        sesion->usuario = (char *)malloc(n*sizeof(char));
        strcpy(sesion->usuario,buff);
    }else{
        return -1;
    }

    n = mjson_get_string(json,len,"$.creacion",buff,sizeof(buff));
    if(n > 0){
        sesion->creacion = atol(buff);
    }else{
        return -1;
    }

    return 1;
}

/******************************************************************
* @brief: Busca una sesion por id en el repositorio JSON y los
* datos encontrados los llena en una estructura
* @param: Id de sesión y estructura donde se llenarán los datos.
* @return: 1 si la estructura se llenó correctamente, -1 si no.												
******************************************************************/
int buscar_sesion_por_id(int id, struct datos_sesion *sesion){
    FILE *fp;
    char c,buff[200],id_str[11],id_enc[11];
    int i=0,len=0,n;
    // long int puntero_inicio,puntero_fin;

    sprintf(id_str,"%d",id);

    fp = fopen("sesiones.json","r");

    if(fp == NULL){
        printf("Hubo un error al abrir el archivo");
        return -1;        
    }
    else{
        while(c!=EOF){
            while(i<200){
                    c=fgetc(fp);                            
                    if(c=='{'){                      
                        // puntero_inicio = ftell(fp);
                        i=0;
                    }
                    if(c!=' ' && c!='\n' && c!='\t'){
                        buff[i] = c;
                        i++;
                    }
                    if(c == '}') {
                        buff[i] = '}';
                        // puntero_fin = ftell(fp);
                        break;
                    }
                    if(c == ']') break;
                }
            if(c == ']') break;

            buff[i] = '\0';
            len = i;
            i=0;

            // printf("%s ** %d\n",buff,len);
            n = mjson_get_string(buff,len,"$.id",id_enc,sizeof(id_enc));
            if (n<0){
                fclose(fp);    
                return -1;
            }
            if(!strcmp(id_str,id_enc)){
                llenar_struct_sesion(sesion,buff,len);
                fclose(fp);
                return 1;
            }
        }
    
    }
    fclose(fp);
    return -1;
}

/******************************************************************
* @brief: Elimina una sesión del repositorio JSON dado un ID.
* @param: Id de sesión.
* @return: 1 si la sesión se eliminó correctamente, -1 si no.												
******************************************************************/
int eliminar_sesion(int id){
    FILE *fp_busqueda,*fp_borrado;
    char c='x',buff[200],id_str[11],id_enc[11];
    int i=0,len=0,n;
    long int puntero_inicio,puntero_fin;

    sprintf(id_str,"%d",id);

    fp_busqueda = fopen("sesiones.json","r");
    fp_borrado = fopen("sesiones.json","r+");


    if(fp_busqueda == NULL){
        printf("Hubo un error al abrir el archivo");
        return -1;
    }
    else{
        while(c!=EOF){
                while(1){
                    c=fgetc(fp_busqueda);
                    if(c=='{'){                      
                        puntero_inicio = ftell(fp_busqueda);
                        i=0;
                    }
                    if(c!=' ' && c!='\n' && c!='\t'){
                        buff[i] = c;
                        i++;
                    }
                    if(c == '}') {
                        buff[i] = '}';
                        while(1){
                            c = getc(fp_busqueda);
                            if(c == ','){
                                puntero_fin = ftell(fp_busqueda);
                                break;
                            }else if(c == ']'){
                                puntero_inicio -= 1;
                                puntero_fin = ftell(fp_busqueda) - 1;
                                c = getc(fp_busqueda);
                                break;
                            }
                        }
                        break;
                    }
                    if(c == ']') break;
                }
            if(c == ']') break;

            buff[i] = '\0';
            len = i;
            i=0;

            //LOG(LL_INFO,("%s ** %d\n",buff,len));
            n = mjson_get_string(buff,len,"$.id",id_enc,sizeof(id_enc));
            if (n<0){
                fclose(fp_busqueda);
                fclose(fp_borrado);
                return -1;
            }

            if(!strcmp(id_str,id_enc)){
                c='\0';
                fseek(fp_busqueda,puntero_inicio-1,SEEK_SET);
                fseek(fp_borrado,puntero_inicio-1,SEEK_SET);
                while(1){
                    c = fgetc(fp_busqueda);
                    fputc(' ',fp_borrado);
                    if(ftell(fp_borrado) == puntero_fin) break;
                }
                fclose(fp_busqueda);
                fclose(fp_borrado);
                return 1;
            }
        }
    }
    
    fclose(fp_busqueda);
    fclose(fp_borrado);
    return -1;
}

/******************************************************************
* @brief: Valida que una sesión este dentro del repositorio JSON.
* @param: Id de sesión y clave SHA de sesión.
* @return: 1 si la sesión es válida, -1 si no.												
******************************************************************/
int validar_sesion(int id, uint64_t sha){
    struct datos_sesion sesion;
    char id_str[11],sha_str[22];
    int n;

    sprintf(id_str,"%d",id);
    sprintf(sha_str,"%llu",sha);
    
    n = buscar_sesion_por_id(id,&sesion);

    if(n>0){
        if(sesion.sha == sha){
            return 1;
        }else{
            LOG(LL_ERROR,("SHA no válido: %llu : %llu\n",sesion.sha,sha));
            return -1;
        }
    }else{
        printf("Sesion no encontrada\n");
        return -1;
    }
    
    return -1;
}

/******************************************************************
* @brief: Convierte una cadena con el formato de una cookie a 
* una cadena con el formato JSON.
* @param: Cadena cookie, longitud de la cadena y un buffer.
* @return: Tamaño de la cadena en formato JSON.												
******************************************************************/
int cookie_a_json(char *cookie, int len, char *buff){
    int i=0,j=0;
    //char buff[len+20];

    buff[0]='{';
    i++;
    buff[1]='"';
    i++;

    for(;j<=len;i++,j++){
        if(cookie[j] == '='){
            buff[i] = '"';
            i++;
            buff[i] = ':';
            i++;
            buff[i]='"';
        }
        else if(cookie[j] == ';'){
            buff[i] = '"';
            i++;
            buff[i] = ',';
        }
        else if(cookie[j] == ' '){
            buff[i] = '"';
        }else{
            buff[i] = cookie[j];
        } 
    }
    // printf("%d -> %d \n",j,i);
    buff[i-1] = '"';
    buff[i] = '}';
    buff[i+1] = '\0';

    return i+1;
}

/******************************************************************
* @brief: Valida si los datos en la cookie mandada por el cliente
* contiene datos válidos para el inicio de sesión.
* @param: Cadena cookie y longitud de la cadena.
* @return: 1 si la cookie contiene datos válidos, -1 si no.
******************************************************************/
int validar_cookie(char *cookie, int len){
    int n,ses_id,len_json;
    uint64_t ses_sha;
    char id[15],sha[22];
    char buff[len+20];

    len_json = cookie_a_json(cookie,len,buff);

    //printf("%s -> %s\n",buff,cookie);
    // LOG(LL_INFO,("Co_ %s\n Buff_ %s\n",cookie,buff));
    n = mjson_get_string(buff,len_json+1,"$.session_id",id,sizeof(id));
    ses_id = atoi(id);

    n = mjson_get_string(buff,len_json+1,"$.session_sha",sha,sizeof(sha));
    ses_sha = atol(sha);
    // printf("ID_ %d SHA_ %llu\n",ses_id,ses_sha);
    
    n = validar_sesion(ses_id,ses_sha);
    
    
    if (n > 0){
        LOG(LL_INFO,("Sesión válida"));
        return 1;
    }
    else{
        LOG(LL_INFO,("Sesión no válida"));
        return -1;
    }
}

/******************************************************************
* @brief: Busca el id contenido dentro de una cadena en formato de
* cookie.
* @param: Cadena cookie y longitud de la cadena.
* @return: ID encontrado en los datos de la cookie, -1 hubo algún
* error.
******************************************************************/
int buscar_id_sesion(char *cookie, int len){
    char buff[len+20],id[15];
    int n,ses_id,len_json;

    len_json = cookie_a_json(cookie,len,buff);
    n = mjson_get_string(buff,len_json,"$.session_id",id,sizeof(id));
    if(n<0){
        return -1;
    }
    ses_id = atoi(id);

    return ses_id;
}

int buscar_nombre_sesion(char *cookie, int len, char *buffer){
    char buff[len+20],nombre[50];
    int n,len_json;

    len_json = cookie_a_json(cookie,len,buff);
    n = mjson_get_string(buff,len_json,"$.session_user",nombre,sizeof(nombre));
    if(n<0){
        return -1;
    }
    strcpy(buffer,nombre);
    return n;
}