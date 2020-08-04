CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic 

all : lsfun.o server.o client.o request.o queue.o

server.o: server.c lsfun.o request.o queue.o
	$(CC) $(CFLAGS) server.c lsfun.o request.o queue.o -o server.o -lpthread

client.o: client.c
	$(CC) $(CFLAGS)  client.c -o client.o

lsfun.o: lsfun.c
	$(CC) -c lsfun.c -o lsfun.o

request.o: request.c
	$(CC) -c request.c -o request.o

queue.o: queue.c
	$(CC) -c queue.c -o queue.o

test: test.c lsfun.o
	$(CC) $(CFLAGS) test.c lsfun.o -o test.o
clean:
	rm -f server.o client.o lsfun.o a.out lsfun.a test.o queue.o request.o
