#include "mongoose.h"
#include "estructuras.h"
#include <stdio.h>
#include <stdlib.h>

/******************************************************************
* @brief: Rutuina encargada de desempaquetar la trama de comunicación
* TCP y obtener los datos
* @param: Trama recibida y estructura donde se guardarán los datos
* desempaquetados                                                      
* @return: NINGUNO												
******************************************************************/
void procesar_cadena(char *str,struct datos_recibidos *dr){
  int i,j=0,k=0;
  char aux[10];

  for(i=0;;i++){
    aux[j] = str[i];
    // LOG(LL_INFO, ("%c ",str[i]));

    j++;
    if(str[i] == ',' || str[i] == '\0'){
      aux[j-1]='\0';
      j=0;
      // LOG(LL_INFO,("k: %d\naux: %s\n",k,aux));

      switch (k)
      {
      case 0:
        dr->medicion_gas = atof(aux);
        // LOG(LL_INFO,("Gas: %s",aux));
        break;
      case 1:
        dr->medicion_temp = atof(aux);
        // LOG(LL_INFO,("Temp: %s",aux));
        break;
      case 2:
        dr->medicion_hum = atof(aux);
        // LOG(LL_INFO,("Humedad: %s",aux));
        break;
      case 3:
        dr->bandera_alerta = atoi(aux);
        // LOG(LL_INFO,("Bandera: %s",aux));
        break;
      }
      k++;
    }

    if(str[i] == '\0')
      break;
  }
}

/******************************************************************
* @brief: Rutina que se encargará de escribir los datos en el 
* repositorio
* @param: Número de nodo sensor y estructura con los datos recibidos
* previamente desempaquetados                                                     
* @return: NINGUNO												
******************************************************************/
void escribir_medidas(int no_nodo, struct datos_recibidos dr){
    FILE *fp;
    char texto[50];
    char nombre[20];
    
    sprintf(texto,"{\ngas:%.2f,\ntemp:%.2f,\nhum:%.2f,\nalerta:%d\n}",dr.medicion_gas,dr.medicion_temp,dr.medicion_hum,dr.bandera_alerta);
    sprintf(nombre,"nodo%d.txt",no_nodo);
    fp = fopen(nombre,"w");

    if(fp == NULL)
        printf("Hubo un error al abrir el archivo");
    else
        fputs(texto,fp);
    fclose(fp);
}