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
  double hum,gas,temp;

  sprintf(nombre,"nodo%d.json",no_nodo);
  
  pthread_mutex_lock(&sensores_lock);

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
    // printf("%s ** %d\n",buff,len);
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
  }
    
  fclose(fp);
  return 1;
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

    sprintf(buffer,"{\n\t\"temp\":%.2f,\n\t\"hum\":%.2f,\n\t\"gas\":%.2f\n}", r.medicion_temp, r.medicion_hum, r.medicion_gas);
    sprintf(nombre,"nodo%d.json",no_nodo);
    fd = open(nombre, O_WRONLY|O_TRUNC|O_CREAT, 0666);
    if( fd == -1 ){
      printf("Hubo un error al abrir el archivo\n");
      return 0;
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