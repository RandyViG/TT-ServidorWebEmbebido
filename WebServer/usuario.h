int llenar_struct_usuario(struct datos_usuario *datos,char *json, int len);
int obtener_usuario_por_indice(int index, struct datos_usuario *datos); //usuario[indice]
int validar_usuario(char *usr, char *pwd);
int verificar_administrador(char *nombre, int len);
int agregar_usuario(char *nombre, char *psw, char *email, int nodo, int admin );
int eliminar_usuario(char *nombre);
int actualizar_usuario_admin(char *nombre_ant, char *n_nombre, char *n_psw, char *n_email,int n_id_nodo);
int buscar_usuario_por_nombre(char *nombre, struct datos_usuario *usuario);
