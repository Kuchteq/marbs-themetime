CFLAGS=-O2 -Wall -Wextra -Wpedantic -lm
PREFIX ?= /usr/local

all: themetime
themetime: themetime.c lib/locationprobe.c lib/sunriset.c
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f themetime

install: themetime
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 0755 themetime $(DESTDIR)$(PREFIX)/bin/themetime

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/themetime
