PROYECTO=servidor_web_embebido
OBJECTS:=$(patsubst %.c,%.o,$(wildcard *.c))
DESTDIR=/usr/bin
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

uninstall:
	rm $(DESTDIR)/$(PROYECTO)

daemon:
	if [ ! -f $(DAEMONDIR)/$(DAEMON_SCRIPT) ]; then \
		cp $(DAEMON_SCRIPT) $(DAEMONDIR); \
	fi; \
	systemctl start servidor_web_embebido.service
	systemctl enable servidor_web_embebido.service

clean: 
	rm -f *.o $(PROYECTO)

