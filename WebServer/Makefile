PROYECTO=servidor_web_embebido
OBJECTS:=$(patsubst %.c,%.o,$(wildcard *.c))
DESTDIR=/usr/local/bin
DAEMONDIR=/etc/systemd/system
DAEMON_SCRIPT=servidor_web_embebido.service
CFLAGS=-Wall
OPTIONS=-DMG_ENABLE_DIRECTORY_LISTING=1 -DMG_ENABLE_OPENSSL=1 -I$(/usr/bin/openssl)/include
LFLAGS=-lm -lpthread -lssl -lcrypto
CC=gcc

all: $(PROYECTO)

%.o: %.c
	${CC} -c $< $(OPTIONS) $(CFLAGS)

$(PROYECTO): ${OBJECTS}
	$(CC) $^ -o $@ $(LFLAGS)

install: all 
	if [ ! -d $(DESTDIR) ]; then \
		sudo mkdir $(DESTDIR); \
	fi; \
	sudo cp $(PROYECTO) $(DESTDIR)

uninstall:
	sudo rm $(DESTDIR)/$(PROYECTO)

daemon:
	if [ ! -f $(DAEMONDIR)/$(DAEMON_SCRIPT) ]; then \
		sudo cp $(DAEMON_SCRIPT) $(DAEMONDIR); \
	fi; \
	sudo systemctl enable servidor_web_embebido.service
	sudo systemctl start servidor_web_embebido.service

clean: 
	rm -f *.o $(PROYECTO)

