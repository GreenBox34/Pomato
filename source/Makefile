CC      = cc
CFLAGS  = -Wall -Wextra -Werror -m64
INCLUDE = `pkg-config --cflags raylib libnotify`
LDLIBS  = `pkg-config --libs raylib libnotify`
OBJECTS = pomato.o
LDFLAGS =

pomato: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

pomato.o: pomato.c
	$(CC) -c pomato.c $(CFLAGS) $(INCLUDE)

install: pomato
	mkdir -p /usr/local/bin
	cp -v pomato /usr/local/bin
	chmod 755 /usr/local/bin/pomato

uninstall:
	rm -fv /usr/local/bin/pomato

run: pomato
	./pomato

clean:
	rm -f pomato *.o

.PHONY: run clean install uninstall
