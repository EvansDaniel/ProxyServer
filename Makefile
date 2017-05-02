CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: server

server: server
	$(CC) $(CFLAGS) -o server server.c csapp.c log.c parser_headers.c $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

#clean:
#	rm -f *.o file.txt

