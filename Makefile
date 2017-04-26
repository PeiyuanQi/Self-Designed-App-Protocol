CC=gcc
CFLAGS=-std=c11 -Dunix=1 -Dlinux=1 -U__STRICT_ANSI__ -g -Wall -Werror

all: proj4 proj4d

multi:multiserver proj4

proj4: client.c bulletin.c bulletin.h client.h
	$(CC) $(CFLAGS) -o $@ bulletin.c client.c

proj4d: server.c bulletin.c bulletin.h server.h
	$(CC) $(CFLAGS) -o $@ bulletin.c  server.c

multiserver: server_multi.c bulletin.c bulletin.h
	$(CC) $(CFLAGS) -o proj4d server_multi.c bulletin.c

clean:
	rm -f *.o
	rm -f proj4 proj4d
	rm -fr *.dSYM






