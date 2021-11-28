int agregar_sesion(char *usuario,struct datos_sesion *sesion);
int buscar_sesion_por_id(int id, struct datos_sesion *sesion);
int validar_sesion(int id, uint64_t sha);
int validar_cookie(char *cookie, int len);
int cookie_a_json(char *cookie, int len, char *buff);
int buscar_sesion_por_id(int id, struct datos_sesion *sesion);
int buscar_id_sesion(char *cookie, int len);
int buscar_nombre_sesion(char *cookie, int len, char *buffer);
int eliminar_sesion(int id);
