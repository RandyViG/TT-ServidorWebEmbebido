#include <stdio.h>
#include <stdlib.h>

#include "estructuras.h"
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
    double nodo;

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
        datos->nodo = (int)nodo;
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
                mjson_get_string(buff,len,"$.user",buff_psw,sizeof(buff_psw));
                // LOG(LL_INFO,("usr: %s: psw: %s\n",buff_usr,buff_psw));
                if(strcmp(buff_usr,usr) == 0 && strcmp(buff_psw,pwd) == 0){
                    return 1;
                }
            }                
        }
    }
    return -1;
}