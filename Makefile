CC=gcc
CFLAGS=-g -Wall -Werror

all: proj4 proj4d

proj4: client.c bulletin.h client.h
	$(CC) $(CFLAGS) -o $@ $<

proj4d: server.c bulletin.h server.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o
	rm -f proj2






