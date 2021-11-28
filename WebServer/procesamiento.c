#include "mongoose.h"
#include "estructuras.h"
#include "mjson.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

extern pthread_mutex_t sensores_lock;

/******************************************************************
* @brief: Rutina que se encargará de leer los datos del 
* repositorio
* @param: Número de nodo sensor, estructura con los datos donde se
* almacenarán los datos encontrados.
* @return: 1 si se escribió correctamente, 0 si no												
******************************************************************/
int leer_medidas(int no_nodo, struct datos_recibidos *datos){
  FILE *fp;
  int i,len,n;
  char c='x',nombre[30],buff[300];
  double hum,gas,temp,bandera;

  sprintf(nombre,"nodo%d.json",no_nodo);
  
  pthread_mutex_lock(&sensores_lock);

  fp = fopen(nombre,"r");
  if(fp == NULL){
    printf("Hubo un error al abrir el archivo\n");
    return -1;
  }
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
    // printf("WS: %s ** %d\n",buff,len);
    pthread_mutex_unlock(&sensores_lock);

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
    n = mjson_get_number(buff,len,"$.incidente",&bandera);
    if (n<0){
      fclose(fp);    
      return -1;
    }else{
      datos->alerta = (time_t)bandera;
    }
  }
    
  fclose(fp);
  return 1;
}

int enviar_correo(char *mail, char *usr){
    char cmd[300];

    sprintf(cmd,"./sendmail.exp \"%s\" \"%s\" ",mail,usr);
    printf("%s",mail);
    return system(cmd);
}

int enviar_correo_a_usuarios_nodo(int no_nodo){
    int i=1,len,n1,n2,n3;
    FILE *fp;
    char c,buff[200],buff_mail[200],buff_usr[100];
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
                n3 = mjson_get_string(buff,len,"$.user",buff_mail,sizeof(buff_usr));

                if(n1 < 0 || n2 < 0 || n3 < 0) return -1;
                if((int)nodo == no_nodo){
                  enviar_correo(buff_mail,buff_usr);
                  LOG(LL_INFO,("Correo enviado a %s para el nodo %d\n",buff_mail,(int)nodo));
                  //printf("Correo enviado a %s para el nodo %d\n",buff_mail,(int)nodo);
                }
            }                
        }
        return 0;
    }
    return -1;
}

/******************************************************************
* @brief: Rutina que se encargará de escribir los datos en el 
* repositorio
* @param: Número de nodo sensor, estructura con los datos recibidos
* previamente desempaquetados y id del sensor al que se escribirá
* el valor                                                
* @return: 1 si se escribió correctamente, 0 si no												
******************************************************************/
int escribir_medidas(int no_nodo,struct datos_recibidos dr,int id_sensor){
    struct datos_recibidos r;
    char buffer[100], nombre[30];
    int fd;

    memset(buffer, 0, 100);
    leer_medidas( no_nodo, &r );

    switch( id_sensor ){
      case 0:
        r.medicion_temp = dr.medicion_temp;
      break;
      case 1:
        r.medicion_hum = dr.medicion_hum;
      break;
      case 2:
        r.medicion_gas = dr.medicion_gas;
      break;
    }

    sprintf(buffer,"{\n\t\"temp\":%.2f,\n\t\"hum\":%.2f,\n\t\"gas\":%.2f,\n\t\"alerta\":%ld\n}", r.medicion_temp, r.medicion_hum, r.medicion_gas, dr.alerta);
    sprintf(nombre,"nodo%d.json",no_nodo);
    fd = open(nombre, O_WRONLY|O_TRUNC|O_CREAT, 0666);
    if( fd == -1 ){
      printf("Hubo un error al abrir el archivo\n");
      return -1;
    }
    write( fd, buffer, strlen(buffer) );
    close(fd);

    return 1;
}

/******************************************************************
* @brief: Rutuina encargada de desempaquetar la trama de comunicación
* TCP y obtener los datos
* @param: Trama recibida y estructura donde se guardarán los datos
* desempaquetados                                                      
* @return: NINGUNO												
******************************************************************/
void procesar_cadena(unsigned char *str,struct datos_recibidos *dr){
  int id_nodo,medida;
  short int id_sensor;
  struct datos_recibidos recuperar;
  float vo_cas, vo_sensor, humedad, temperatura;
  time_t tiempo_incidencia;

  id_nodo = str[0];
  id_nodo = id_nodo << 8;
  id_nodo = id_nodo | str[1];

  id_sensor = str[2];

  medida = str[3];
  medida = medida << 8;
  medida = medida | str[4];

  /*ID_SENSOR: 0 = TEMPERATURA*/
  /*ID_SENSOR: 1 = HUMEDAD*/
  /*ID_SENSOR: 2 = GAS*/

  LOG(LL_INFO,("ID_NODO: %d",id_nodo));
  LOG(LL_INFO,("ID_SENSOR: %d",id_sensor));

  switch (id_sensor){
    case 0:
      dr->medicion_temp = 175.0 * ( ( (float)medida)/65535.0 ) - 45.0;
      LOG(LL_INFO,("MEDIDA: %.2f",dr->medicion_temp));
      break;
    case 1:
      dr->medicion_hum = 100.0 * ( ( (float)medida)/65535.0 );
      LOG(LL_INFO,("MEDIDA: %.2f",dr->medicion_hum));
      break;
    case 2:
      vo_cas = (float)medida * (3.3/4096.0);
      vo_sensor = vo_cas / 8.0;
      leer_medidas( id_nodo, &recuperar );
      humedad = recuperar.medicion_hum;
      temperatura = recuperar.medicion_temp;
      dr->medicion_gas = vo_sensor / ( ( (2/45)*humedad + (1/20)*temperatura + 25.3 ) * 0.001 ); 
      LOG(LL_INFO,("MEDIDA: %.2f",dr->medicion_gas));

      /*Verificación de tiempo para correo*/
      if(dr->medicion_gas >= 1.5 && dr->medicion_gas <= 2.5){
        tiempo_incidencia = time(0) / 60;
        if(recuperar.alerta - tiempo_incidencia > 1){
          enviar_correo_a_usuarios_nodo(id_nodo);
        }
      }
    default:
      break;
  }
  
  escribir_medidas(id_nodo,*dr,id_sensor);
}

int str_len(char *str){
    int i=0;

    while(str[i]!='\0'){
        // printf("%c",str[i]);
        i++;
    }
    return i+1;
}