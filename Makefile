CC=gcc
CFLAGS=-g -Wall -Werror

all: proj4 proj4d

proj4: client.c bulletin.c bulletin.h client.h
	$(CC) $(CFLAGS) -o $@ client.c bulletin.c

proj4d: server.c bulletin.c bulletin.h server.h
	$(CC) $(CFLAGS) -o $@ server.c bulletin.c

clean:
	rm -f *.o
	rm -f proj4 proj4d
	rm -f *.dSYM






