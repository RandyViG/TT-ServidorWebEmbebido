#include "mongoose.h"
#include "estructuras.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>

/******************************************************************
* @brief: Rutina que se encargará de escribir los datos en el 
* repositorio
* @param: Número de nodo sensor, estructura con los datos recibidos
* previamente desempaquetados y id del sensor al que se escribirá
* el valor                                                
* @return: NINGUNO												
******************************************************************/
void escribir_medidas(int no_nodo,struct datos_recibidos dr,int id_sensor){
    FILE *fp,*fpw;
    char s2[60];
    char nombre[30];
    char s1[100];
    char c;
    int i=0,j=0;
    char *res = NULL,json[300];

    switch (id_sensor)
    {
    case 1:
      sprintf(s2,"{\"temp\":%.2f}",dr.medicion_temp);
      break;
    case 2:
      sprintf(s2,"{\"hum\":%.2f}",dr.medicion_hum);
      break;
    case 3:
      sprintf(s2,"{\"gas\":%.2f}",dr.medicion_gas);
    default:
      break;
    }
    sprintf(nombre,"nodo%d.json",no_nodo);

    fp = fopen(nombre,"r");

    if(fp == NULL)
        printf("Hubo un error al abrir el archivo");
    else{
      while(c!=EOF){
        c=getc(fp);
        s1[i] = c;
        i++;
      }
      if(i<3){
        i=3;
        s1[0]='{';
        s1[1]='}';
        s1[3]='\0';
      }
    }

    mjson_merge(s1,i,s2,strlen(s2),&mjson_print_dynamic_buf,&res);
    
    /*Para no imprimir en una sola linea el json*/
    i=0;
    j=0;
    while(res[i] != '\0'){
      if(res[i] == '{' || res[i] == ','){
        json[j]=res[i];
        j++;
        json[j]='\n';
        j++;
        json[j]='\t';
        j++;
        i++;
      }
      else if(res[i]=='}'){
        json[j]='\n';
        j++;
        json[j]='}';
        j++;
        json[j]='\0';
        break;
      }
      else{
        json[j] = res[i];
        j++;
        i++;
      }
    }


    fpw = fopen(nombre,"w");
    if(fpw == NULL)
        printf("Hubo un error al abrir el archivo");
    else
      fputs(json,fpw);
    fclose(fpw);
    fclose(fp);
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

  id_nodo = str[0];
  id_nodo = id_nodo << 8;
  id_nodo = id_nodo | str[1];

  id_sensor = str[2];

  medida = str[3];
  medida = medida << 8;
  medida = medida | str[4];

  /*ID_SENSOR: 1 = TEMPERATURA*/
  /*ID_SENSOR: 2 = HUMEDAD*/
  /*ID_SENSOR: 3 = GAS*/

  LOG(LL_INFO,("ID_NODO: %d",id_nodo));
  LOG(LL_INFO,("ID_SENSOR: %d",id_sensor));

  switch (id_sensor)
  {
  case 1:
    dr->medicion_temp = 175.0 * ( ( (float)medida)/65535.0 ) - 45.0;
    LOG(LL_INFO,("MEDIDA: %.2f",dr->medicion_temp));
    break;
  case 2:
    dr->medicion_hum = 100.0 * ( ( (float)medida)/65535.0 );
    LOG(LL_INFO,("MEDIDA: %.2f",dr->medicion_hum));
    break;
  case 3:
    dr->medicion_gas = 100.0 * ( ( (float)medida)/65535.0 );
    LOG(LL_INFO,("MEDIDA: %.2f",dr->medicion_gas));
  default:
    break;
  }

  escribir_medidas(id_nodo,*dr,id_sensor);
}