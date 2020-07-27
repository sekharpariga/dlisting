CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic 

all : lsfun.o server.o client.o test request.o

server.o: server.c lsfun.o request.o
	$(CC) $(CFLAGS) server.c lsfun.o request.o -o server.o

client.o: client.c
	$(CC) $(CFLAGS)  client.c -o client.o

lsfun.o: lsfun.c
	$(CC) -c lsfun.c -o lsfun.o

request.o: request.c
	$(CC) -c request.c -o request.o

test: test.c lsfun.o
	$(CC) $(CFLAGS) test.c lsfun.o -o test.o
clean:
	rm -f server.o client.o lsfun.o a.out lsfun.a test.o
