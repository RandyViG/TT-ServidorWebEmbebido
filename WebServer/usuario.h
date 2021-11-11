int llenar_struct_usuario(struct datos_usuario *datos,char *json, int len);
int obtener_usuario_por_indice(int index, struct datos_usuario *datos); //usuario[indice]
int validar_usuario(char *usr, char *pwd);