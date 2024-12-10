CC=gcc
CFLAGS= -Wall -Wextra
PGK_CONFIG=`pkg-config --cflags --libs raylib libnotify`

all: pomato

pomato: pomato.c
	$(CC) $(CFLAGS) -o $@ pomato.c $(PGK_CONFIG) 

install:
	strip pomato
	mkdir -p /usr/local/bin
	cp -v pomato /usr/local/bin
	chmod 755 /usr/local/bin/pomato

uninstall:
	rm -v /usr/local/bin/pomato

run: pomato
	./pomato

clean:
	rm -f pomato

.PHONY: all run clean install uninstall

