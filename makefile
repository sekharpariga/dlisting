CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic 

all : lsfun.o server.o client.o test

server.o: server.c lsfun.a
	$(CC) $(CFLAGS) server.c -o server.o lsfun.a

client.o: client.c
	$(CC) $(CFLAGS)  client.c -o client.o

lsfun.o: lsfun.c
	$(CC) -c lsfun.c -o lsfun.o

lsfun.a: lsfun.o
	ar rcs lsfun.a lsfun.o
test: test.c lsfun.o
	$(CC) $(CFLAGS) test.c lsfun.o -o test.o
clean:
	rm -f server.o client.o lsfun.o a.out lsfun.a test.o
