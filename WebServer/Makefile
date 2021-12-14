PROYECTO=servidor_web_embebido
OBJECTS:=$(patsubst %.c,%.o,$(wildcard *.c))
DESTDIR=/usr/bin
NODODIR=/home/root/nodos
SESSIONDIR=/home/root/.session
CERTSDIR=/home/root/.certs
PUBLICDIR=/home/root/public
DAEMONDIR=/etc/systemd/system
DAEMON_SCRIPT=servidor_web_embebido.service
CFLAGS=-Wall
OPTIONS=-DMG_ENABLE_DIRECTORY_LISTING=1 
LFLAGS=-lm -lpthread
CC=gcc

all: $(PROYECTO)

%.o: %.c
	${CC} -c $< $(OPTIONS) $(CFLAGS)

$(PROYECTO): ${OBJECTS}
	$(CC) $^ -o $@ $(LFLAGS)

install: all 
	if [ ! -d $(DESTDIR) ]; then \
		mkdir $(DESTDIR); \
	fi; \
	cp $(PROYECTO) $(DESTDIR)
	if [ ! -d $(NODODIR) ]; then \
		mkdir $(NODODIR); \
	fi; \
	if [ ! -d $(SESSIONDIR) ]; then \
		mkdir $(SESSIONDIR); \
	fi; \
	if [ ! -d $(CERTSDIR) ]; then \
		mkdir $(CERTSDIR); \
	fi; \
	cp /home/root/WebServer/cert.pem $(CERTSDIR)
	cp /home/root/WebServer/key.pem	$(CERTSDIR)
	if [ ! -d $(PUBLICDIR) ]; then \
		mkdir $(PUBLICDIR); \
	fi; \
	cp -r /home/root/WebServer/public/* $(PUBLICDIR)
	cp -r /home/root/WebServer/sendmail.exp /home/root/
	cp /home/root/WebServer/usuarios.json.example /home/root/.session/usuarios.json
	cp /home/root/WebServer/sesiones.json.example /home/root/.session/sesiones.json
	cp /home/root/WebServer/nodo1.json.example /home/root/nodos/nodo1.json

uninstall:
	rm -r $(NODODIR)
	rm -r $(SESSIONDIR)
	rm -r $(CERTSDIR)
	rm -r $(PUBLICDIR)
	rm $(DAEMONDIR)/$(DAEMON_SCRIPT)
	rm $(DESTDIR)/$(PROYECTO)

daemon:
	if [ ! -f $(DAEMONDIR)/$(DAEMON_SCRIPT) ]; then \
		cp $(DAEMON_SCRIPT) $(DAEMONDIR); \
	fi; \
	systemctl start servidor_web_embebido.service
	systemctl enable servidor_web_embebido.service

clean: 
	rm -f *.o $(PROYECTO)

