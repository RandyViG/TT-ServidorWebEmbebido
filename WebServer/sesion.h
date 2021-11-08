int llenar_struct_usuario(struct datos_usuario *datos,char *json, int len);
int obtener_usuario_por_indice(int index, struct datos_usuario *datos); //usuario[indice]
int validar_usuario(char *usr, char *pwd);
int agregar_sesion(char *usuario,struct datos_sesion *sesion);
int validar_sesion(int id, uint64_t sha);
int validar_cookie(char *cookie, int len);
int cookie_a_json(char *cookie, int len, char *buff);
int buscar_id_sesion(char *cookie, int len);
int eliminar_sesion(int id);
