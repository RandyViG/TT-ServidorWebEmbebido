#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mjson.h"
#include "mongoose.h"

// Estructura útil para guardar los datos de un usuario obtenidos desde usuarios.json
struct datos_usuario
{
    char *nombre;
    char *password;
    char *email;
    short int nodo;
    short int admin;
};

struct datos_sesion{
    int id;
    uint64_t sha;
    char *usuario;
    time_t creacion;
};

struct datos_recibidos{
    float medicion_gas;
    float medicion_temp;
    float medicion_hum;
    int bandera_alerta;
};

struct lista_ws{
    int puerto;
    int tid_ws;
    struct lista_ws *sig;
};

//Agrega una sesión nueva en el repo de sesiones.json que se relacionará a un usuario
int agregar_sesion(char *usuario,struct datos_sesion *sesion){
                // fprintf(fpr,"\n\t{\n\t\"id\":\"%d\",\n\t\"sha\":\"%lu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n]",id,*(uint64_t *) dig,usuario,creado_el);

    FILE *fpr;
    char c;

    int id,len_usr,i=0,flag=0;
    time_t creado_el = time(0);
    char id_str[10];
    mg_sha1_ctx ctx;
    unsigned char dig[20];

    srand(time(NULL)*clock());
    id = rand();

    sprintf(id_str,"%d",id);
    mg_sha1_init(&ctx);
    mg_sha1_update(&ctx,(unsigned char *)id_str,10);
    mg_sha1_update(&ctx,(unsigned char *)"CHSMPP",7);
    mg_sha1_final(dig,&ctx);

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
                    fprintf(fpr,"[\n\t{\n\t\"id\":\"%d\",\n\t\"sha\":\"%lu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n]",id,*(uint64_t *) dig,usuario,creado_el);
                    break;
                }
                flag++;
            }
            if(c == '['){
                printf("%d",i);

                if(i<108)
                    fprintf(fpr,"\n\t{\n\t\"id\":\"%d\",\n\t\"sha\":\"%lu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n]",id,*(uint64_t *) dig,usuario,creado_el);
                else
                    fprintf(fpr,"\n\t{\n\t\"id\":\"%d\",\n\t\"sha\":\"%lu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n",id,*(uint64_t *) dig,usuario,creado_el);
                break;
            }    
            if(c == '}'){
                printf("%d",i);
                if(i<108)                
                    fprintf(fpr,"\n\t,{\n\t\"id\":\"%d\",\n\t\"sha\":\"%lu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n]",id,*(uint64_t *) dig,usuario,creado_el);
                else
                    fprintf(fpr,"\n\t,{\n\t\"id\":\"%d\",\n\t\"sha\":\"%lu\",\n\t\"usuario\":\"%s\",\n\t\"creacion\":\"%ld\"\n\t}\n",id,*(uint64_t *) dig,usuario,creado_el);
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

            printf("%s ** %d\n",buff,len);
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
    return -1;
}

int validar_sesion(int id, uint64_t sha){
    struct datos_sesion sesion;
    char id_str[11],sha_str[22];
    int n;

    sprintf(id_str,"%d",id);
    sprintf(sha_str,"%lu",sha);

    n = buscar_sesion_por_id(id,&sesion);
    if(n>0){
        if(sesion.sha == sha){
            return 1;
        }else{
            printf("SHA no válido: %lu : %lu\n",sesion.sha,sha);
            return -1;
        }
    }else{
        printf("Sesion no encontrada\n");
        return -1;
    }
    
    return -1;
}

int validar_cookie(char *cookie, int len){
    int i=0,j=0,n,ses_id;
    uint64_t ses_sha;
    char buff[len+20];
    char id[15],sha[22];

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
    printf("%d -> %d \n",j,i);
    buff[i-1] = '"';
    buff[i] = '}';
    buff[i+1] = '\0';

     printf("%s -> %s\n",buff,cookie);
    // LOG(LL_INFO,("Co_ %s\n Buff_ %s\n",cookie,buff));
    n = mjson_get_string(buff,i+1,"$.session_id",id,sizeof(id));
    ses_id = atoi(id);

    n = mjson_get_string(buff,i+1,"$.session_sha",sha,sizeof(sha));
    ses_sha = atol(sha);
    printf("%d %lu\n",ses_id,ses_sha);
    
    n = validar_sesion(ses_id,ses_sha);

    if (n > 0){
        // LOG(LL_INFO,("sesión válida"));
        printf("Sesion válida");
        return 1;
    }
    else{
        printf("Sesion no válida");
        // LOG(LL_INFO,("sesión no válida"));
        return -1;
    }
}

int eliminar_sesion(int id){
    FILE *fp_busqueda,*fp_borrado;
    char c,buff[200],id_str[11],id_enc[11];
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

            printf("%s ** %d\n",buff,len);
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
    return 1;
}

// Función que llena una estructura con los datos obtenidos desde una cadena json
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

//Obtiene una cadena json para obtener los datos de un usuario y almacenarlo en una estructura
int obtener_usuario_por_indice(int index, struct datos_usuario *datos){ //usuario[indice]
    FILE *fp;
    char c,buff[200];
    int usr_cont = -1,i=1,len=0,ld;

    fp = fopen("usuarios.json","r");

    if(fp == NULL){
        printf("Hubo un error al abrir el archivo");
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

    if(ld > 0) return 1; else return 0;
}

//Valida que un usuario y contraseña dados coincidan con los de dentro del repo de datos
int validar_usuario(char *usr, char *pwd){
    int i=1,len;
    FILE *fp;
    char c,buff[200],buff_usr[100],buff_psw[100];

    fp = fopen("usuarios.json","r");

    if(fp == NULL){
        printf("Hubo un error al abrir el archivo");
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
                // printf("%s",buff);
                mjson_get_string(buff,len,"$.user",buff_usr,sizeof(buff_usr));
                mjson_get_string(buff,len,"$.user",buff_psw,sizeof(buff_psw));
                // printf("usr: %s: psw: %s\n",buff_usr,buff_psw);
                if(strcmp(buff_usr,usr) == 0 && strcmp(buff_psw,pwd) == 0){
                    return 1;
                }
            }                
        }
    }
    return -1;
}

int str_len(char *str){
    int i=0;

    while(str[i]!='\0'){
        // printf("%c",str[i]);
        i++;
    }
    return i+1;
}

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
    char c;
    struct datos_usuario usuario;
    int i=0,flag=0,max_len=0,usr_exist;

    fpr=fopen("usuarios.json","r+");

    max_len = str_len(nombre)+str_len(psw)+str_len(email)+2+79;
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
                printf("%d",i);

                if(i<max_len)
                    fprintf(fpr,"\n\t{\n\t\"user\":\"%s\",\n\t\"password\":\"%s\",\n\t\"email\":\"%s\",\n\t\"nodo\":%d,\n\t\"admin\":%d\n\t}\n]",nombre,psw,email,nodo,admin);
                else
                    fprintf(fpr,"\n\t{\n\t\"user\":\"%s\",\n\t\"password\":\"%s\",\n\t\"email\":\"%s\",\n\t\"nodo\":%d,\n\t\"admin\":%d\n\t}\n",nombre,psw,email,nodo,admin);
                break;
            }    
            if(c == '}'){
                printf("%d",i);
                if(i<max_len)                
                    fprintf(fpr,"\n\t,{\n\t\"user\":\"%s\",\n\t\"password\":\"%s\",\n\t\"email\":\"%s\",\n\t\"nodo\":%d,\n\t\"admin\":%d\n\t}\n]",nombre,psw,email,nodo,admin);                    
                else
                    fprintf(fpr,"\n\t,{\n\t\"user\":\"%s\",\n\t\"password\":\"%s\",\n\t\"email\":\"%s\",\n\t\"nodo\":%d,\n\t\"admin\":%d\n\t}\n",nombre,psw,email,nodo,admin);
                break;
            }
        }        
    }

    fclose(fpr);
    return 1;
}

int actualizar_usuario_admin(char *nombre_ant, char *n_nombre, char *n_psw, int n_id_nodo){
    struct datos_usuario usuario;
    int usr_exist,e;

    usr_exist = buscar_usuario_por_nombre(n_nombre,&usuario);
    if(usr_exist > 0){printf("Usuario ya existente %d",usr_exist); return -1;}

    usr_exist = buscar_usuario_por_nombre(nombre_ant,&usuario);
    if(usr_exist < 0){printf("No se encontró usuario %d",usr_exist); return -1;}
    
    e = eliminar_usuario(nombre_ant);
    if(e>0){
        e = agregar_usuario(n_nombre,n_psw,usuario.email,n_id_nodo,usuario.admin);
        if(e > 0) return 1;    
    }else{
        return -1;
    }
    return -1;
}

int verificar_administrador(char *nombre){
    int usr_exist;
    struct datos_usuario usuario;

    usr_exist = buscar_usuario_por_nombre(nombre,&usuario);
    if(usr_exist < 0){printf("No se encontró usuario %d",usr_exist); return -1;}

    // printf("Administrador? %d", usuario.admin);
    return (int)usuario.admin;
}


int leer_medidas(int no_nodo, struct datos_recibidos *datos){
  FILE *fp;
  int i,len,n;
  char c='x',nombre[30],buff[300];
  double hum,gas,temp;

  sprintf(nombre,"nodo%d.json",no_nodo);

  fp = fopen(nombre,"r");
    if(fp == NULL)
      printf("Hubo un error al abrir el archivo\n");
    else{
      while(c != EOF){
        c = getc(fp);
        if(c == '{')
            i=0;
        if(c!=' ' && c!='\n' && c!='\t'){
            buff[i]=c;
            i++;
        }
        if(c == '}'){
            break;
        }
      }
      buff[i] = '\0';
      len = i;
      printf("%s ** %d\n",buff,len);

      n = mjson_get_number(buff,len,"$.hum", &hum);
      if (n<0){
          fclose(fp);    
          return -1;
      }else{
        datos->medicion_hum = (float)hum;
      }
      n = mjson_get_number(buff,len,"$.gas", &gas);
      if (n<0){
          fclose(fp);    
          return -1;
      }else{
        datos->medicion_gas = (float)gas;
      }
      n = mjson_get_number(buff,len,"$.temp", &temp);
      if (n<0){
          fclose(fp);    
          return -1;
      }else{
        datos->medicion_temp = (float)temp;
      }
    }
    
    fclose(fp);
    return 1;
}


int enviar_correo(char *mail){
    char cmd[300];

    sprintf(cmd,"sendmail %s < email.txt",mail);
    printf("%s",mail);
    return system(cmd);
}

int enviar_correo_a_usuarios_nodo(int no_nodo){
    int i=1,len,n1,n2;
    FILE *fp;
    char c,buff[200],buff_mail[200];
    double nodo;

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
                n1 = mjson_get_number(buff,len,"$.nodo",&nodo);
                n2 = mjson_get_string(buff,len,"$.email",buff_mail,sizeof(buff_mail));

                if(n1 < 0 || n2 < 0) return -1;
                if((int)nodo == no_nodo){
                    //enviar_correo(buff_mail);
                //  LOG(LL_INFO,("Correo enviado a %s para el nodo %d\n",buff_mail,(int)nodo));
                    printf("Correo enviado a %s para el nodo %d\n",buff_mail,(int)nodo);
                }
            }                
        }
        return 0;
    }
    return -1;
}


int main(){
    enviar_correo_a_usuarios(1);
    // enviar_correo("infosweb053@gmail.com");
    //printf("%d\n",str_len(""));
    // printf("Admin? %d", verificar_administrador("root2"));

    // struct datos_recibidos datos;
    // leer_medidas(1,&datos);
    // printf("%f %f %f",datos.medicion_gas,datos.medicion_hum,datos.medicion_temp);
    // actualizar_usuario_admin("admin","root","zxc",1);
    // struct datos_usuario usuario;
    // buscar_usuario_por_nombre("admin",&usuario);
    // printf("%s %s %s, %d %d \n",usuario.nombre,usuario.password,usuario.email,usuario.nodo,usuario.admin);
    // eliminar_usuario("root");
    // agregar_usuario("zabdiel","asd","asd",1,0);
    // eliminar_usuario("root2");

    // validar_cookie("session_id=428792257; session_sha=11967441517177441041",54);

    // printf("%d ", validar_sesion(146923529,5906924271797926880));

    // eliminar_sesion(2);
    
    // struct datos_sesion sesion;
    // int ou;

    // ou = buscar_sesion_por_id(2, &sesion);
    // if(ou>0){
    //     printf("%d %ld %s, %ld\n",sesion.id,sesion.sha,sesion.usuario,sesion.creacion);
    // }


    // struct datos_sesion sesion;
    // agregar_sesion("admin",&sesion);
    // printf("\nid:%d\nsha:%lu\ncre:%ld\nusr:%s\n",sesion.id,sesion.sha,sesion.creacion,sesion.usuario);
    
    // if(validar_usuario("admin","admin1")>0) printf("Encontrado!"); else printf("No encontrado :(");


    // struct datos_usuario datos;
    // int ou;
    // ou = obtener_usuario_por_indice(0,&datos);

    // if(ou > 0){
    //     puts(datos.nombre);
    //     puts(datos.password);
    //     puts(datos.email);
    //     printf("%d\n",datos.nodo);
    //     printf("%d\n",datos.admin);
    // }
    
    // return 0;
}