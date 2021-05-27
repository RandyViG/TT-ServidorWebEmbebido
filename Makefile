PROYECTO=server
OBJECTS:=$(patsubst %.c,%.o,$(wildcard *.c))
DESTDIR=/usr/local/bin
CFLAGS=-Wall
OPTIONS=-DMG_ENABLE_DIRECTORY_LISTING=1
LFLAGS=-lm
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

clean: 
	rm -f *.o $(PROYECTO)

