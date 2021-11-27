#include <stdio.h>
#include <stdlib.h>

#include "estructuras.h"
#include "procesamiento.h"
#include "mjson.h"
#include "mongoose.h"

/******************************************************************
* @brief: Función que llena una estructura con los datos obtenidos
* desde una cadena json.
* @param: Estructura donde se llenarán los datos, cadena json y
* longitud de la misma.
* @return: 1 si la estructura se llenó correctamente, -1 si no.												
******************************************************************/

int llenar_struct_usuario(struct datos_usuario *datos,char *json, int len){
    char buff[100];
    int n;
    double nodo,admin;

    n = mjson_get_string(json,len,"$.user",buff,sizeof(buff));
    if(n > 0){
        datos->nombre = (char *)malloc(n*sizeof(char));
        strcpy(datos->nombre,buff);
    }else{
        return -1;
    }

    n = mjson_get_string(json,len,"$.password",buff,sizeof(buff));
    if(n > 0){
        datos->password = (char *)malloc(n*sizeof(char));
        strcpy(datos->password,buff);
    }else{
        return -1;
    }

    n = mjson_get_string(json,len,"$.email",buff,sizeof(buff));
    if(n > 0){
        datos->email = (char *)malloc(n*sizeof(char));
        strcpy(datos->email,buff);
    }else{
        return -1;
    }

    n = mjson_get_number(json,len,"$.nodo", &nodo);
    if(n > 0){
        datos->nodo = (short int)nodo;
    }else{
        return -1;
    }

    n = mjson_get_number(json,len,"$.admin", &admin);
    if(n > 0){
        datos->admin = (short int)admin;
    }else{
        return -1;
    }

    return 1;
}

/******************************************************************
* @brief: Obtiene una cadena json para obtener los datos de un 
* usuario y almacenarlo en una estructura.
* @param: indice que se buscará y estructura donde se llenarán 
* los datos.
* @return: 1 si el usuario se encotró, -1 si no o si hubo algún error.												
******************************************************************/

int obtener_usuario_por_indice(int index, struct datos_usuario *datos){ //usuario[indice]
    FILE *fp;
    char c,buff[200];
    int usr_cont = -1,i=1,len=0,ld;

    fp = fopen("usuarios.json","r");

    if(fp == NULL){
        LOG(LL_INFO,("Hubo un error al abrir el archivo"));
        return -1;
    }
    else{
        while(c!=EOF){
            c = fgetc(fp);

            if(usr_cont == index){
                buff[0] = '{';
                while(1){
                    c=fgetc(fp);
                    if(c!=' ' && c!='\n'){
                        buff[i] = c;
                        i++;
                    }
                    if(c == '}') {
                        buff[i] = '}';
                        break;
                    }
                }
                buff[i] = '\0';
                len = i;
                usr_cont++;
            }
            

            if(c == '{') usr_cont++;
            if (usr_cont > index) break;
        }
    }
    
    ld = llenar_struct_usuario(datos,buff,len);
    fclose(fp);

    if(ld > 0) return 1; else return -1;
}

/******************************************************************
* @brief: Valida que un usuario y contraseña dados coincidan con 
* los de dentro del repo de datos.
* @param: Usuario y contraseña que se validará.
* @return: 1 si el usuario se encotró coincidencia,
* -1 si no o si hubo algún error.												
******************************************************************/

int validar_usuario(char *usr, char *pwd){
    int i=1,len;
    FILE *fp;
    char c,buff[200],buff_usr[100],buff_psw[100];

    fp = fopen("usuarios.json","r");

    if(fp == NULL){
        LOG(LL_INFO,("Hubo un error al abrir el archivo"));
        return -1;
    }
    else{
        while(c!=EOF){
            c = fgetc(fp);
            if(c=='{'){
                buff[0]='{';
                while(1){
                    c=fgetc(fp);
                    if(c!=' ' && c!='\n'){
                        buff[i] = c;
                        i++;
                    }
                    if(c == '}' || c == EOF){
                        buff[i] = '}';
                        break;
                    }
                }
                buff[i] = '\0';
                len = i;
                i=1;
                // LOG(LL_INFO,("%s",buff));
                mjson_get_string(buff,len,"$.user",buff_usr,sizeof(buff_usr));
                mjson_get_string(buff,len,"$.password",buff_psw,sizeof(buff_psw));
                // LOG(LL_INFO,("usr: %s: psw: %s\n",buff_usr,buff_psw));
                if(strcmp(buff_usr,usr) == 0 && strcmp(buff_psw,pwd) == 0){
                    return 1;
                }
            }                
        }
    }
    return -1;
}

// int str_len(char *str){
//     int i=0;

//     while(str[i]!='\0'){
//         // printf("%c",str[i]);
//         i++;
//     }
//     return i+1;
// }

long int buscar_coma(long int pt_ini){
    FILE *fp;
    int i=0;
    char c;
    
    fp = fopen("usuarios.json","r");
    if(fp == NULL){
        printf("Hubo un error al abrir el archivo");
        return -1;
    }

    while(1){
        fseek(fp,pt_ini-i,SEEK_SET);
        c = fgetc(fp);
        printf("%c",c);
        if(c == ',' || c == '[' || c == EOF)
            break;
        i++;
    }
    fclose(fp);
    if(c==',')
        return i;
    else
        return 1;
}

int eliminar_usuario(char *nombre){
    FILE *fp_busqueda,*fp_borrado;
    char c,buff[200],usr_enc[50];
    int i=0,len=0,n;
    long int puntero_inicio,puntero_fin;

    fp_busqueda = fopen("usuarios.json","r");
    fp_borrado = fopen("usuarios.json","r+");

    if(fp_busqueda == NULL){
        printf("Hubo un error al abrir el archivo");
        return -1;
    }
    else{
        while(c!=EOF){
                while(i<199){
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
                                puntero_inicio -= buscar_coma(puntero_inicio);
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

            printf("%s ** %d\n",buff,len);

            n = mjson_get_string(buff,len,"$.user",usr_enc,sizeof(usr_enc));
            if (n<0){
                fclose(fp_busqueda);
                fclose(fp_borrado);
                return -1;
            }

            if(!strcmp(nombre,usr_enc)){
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
    return 1;
}

int buscar_usuario_por_nombre(char *nombre, struct datos_usuario *usuario){
    FILE *fp;
    char c,buff[400],nombre_enc[50];
    int i=0,len=0,n;
    // long int puntero_inicio,puntero_fin;

    fp = fopen("usuarios.json","r");

    if(fp == NULL){
        printf("Hubo un error al abrir el archivo");
        return -1;        
    }
    else{
        while(c!=EOF){
            while(i<400){
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

            printf("%s ** %d\n",buff,len);
            n = mjson_get_string(buff,len,"$.user",nombre_enc,sizeof(nombre_enc));
            if (n<0){
                fclose(fp);    
                return -1;
            }
            if(!strcmp(nombre,nombre_enc)){
                llenar_struct_usuario(usuario,buff,len);
                fclose(fp);
                return 1;
            }
        }
    
    }
    return -1;
}

int agregar_usuario(char *nombre, char *psw, char *email, int nodo, int admin ){
    FILE *fpr;
    char c,str_put[600];
    struct datos_usuario usuario;
    int i=0,flag=0,max_len=0,usr_exist;

    fpr=fopen("usuarios.json","r+");

    // max_len = str_len(nombre)+str_len(psw)+str_len(email)+71;
    // printf("max: %d ",max_len);

    usr_exist = buscar_usuario_por_nombre(nombre,&usuario);
    if(usr_exist > 0){printf("Usuario ya existente %d",usr_exist); return -1;}

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
                    fprintf(fpr,"[\n\t{\n\t\"user\":\"%s\",\n\t\"password\":\"%s\",\n\t\"email\":\"%s\",\n\t\"nodo\":%d,\n\t\"admin\":%d\n\t}\n]",nombre,psw,email,nodo,admin);
                    break;
                }
                flag++;
            }
            if(c == '['){
                sprintf(str_put,"\n\t{\n\t\"user\":\"%s\",\n\t\"password\":\"%s\",\n\t\"email\":\"%s\",\n\t\"nodo\":%d,\n\t\"admin\":%d\n\t}",nombre,psw,email,nodo,admin);
                max_len = str_len(str_put) + 1;
                LOG(LL_INFO,("Tamaño de i: %d, len: %d",i,max_len));

                if(i-1<=max_len)
                    fprintf(fpr,"%s\n]",str_put);
                else
                    fprintf(fpr,"%s\n",str_put);
                break;
            }    
            if(c == '}'){
                sprintf(str_put,"\n\t,{\n\t\"user\":\"%s\",\n\t\"password\":\"%s\",\n\t\"email\":\"%s\",\n\t\"nodo\":%d,\n\t\"admin\":%d\n\t}",nombre,psw,email,nodo,admin);
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

    fclose(fpr);
    return 1;
}

int actualizar_usuario_admin(char *nombre_ant, char *n_nombre, char *n_psw, char *n_email,int n_id_nodo){
    struct datos_usuario usuario;
    int usr_exist,e;
    char usr[50],email[50];
    if(strlen(n_nombre) > 1){
        usr_exist = buscar_usuario_por_nombre(n_nombre,&usuario);
        if(usr_exist > 0){LOG(LL_ERROR,("Usuario ya existente %d",usr_exist)); return -1;}
    }

    usr_exist = buscar_usuario_por_nombre(nombre_ant,&usuario);
    if(usr_exist < 0){LOG(LL_ERROR,("No se encontró usuario %d",usr_exist)); return -1;}
    
    if(strlen(n_nombre) <= 1){
        sprintf(usr,"%s",usuario.nombre);
        LOG(LL_INFO,("Nombre vacio"));
    }
    else
        sprintf(usr,"%s",n_nombre);

    if(strlen(n_email) <= 1){
        sprintf(email,"%s",usuario.email);
        LOG(LL_INFO,("email vacio"));
    }
    else
        sprintf(email,"%s",n_email);

    e = eliminar_usuario(nombre_ant);
    if(e>0){
        e = agregar_usuario(usr,n_psw,email,n_id_nodo,usuario.admin);
        if(e > 0) return 1; else return -1;  
    }else{
        LOG(LL_ERROR,("No se pudo eliminar a %s",nombre_ant));
        return -1;
    }
    return -1;
}

int verificar_administrador(char *nombre, int len){
    int usr_exist;
    char buff[50];
    struct datos_usuario usuario;

    strncpy(buff,nombre,len);
    buff[len+1]='\0';

    usr_exist = buscar_usuario_por_nombre(buff,&usuario);
    if(usr_exist < 0){printf("No se encontró usuario %d",usr_exist); return -1;}
    return (int)usuario.admin;
}

 