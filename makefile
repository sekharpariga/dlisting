CC=gcc
CFLAGS=-g -Wall -Wextra -pedantic 

all: server.o client.o common.o queue.o

server.o: server.c common.o queue.o
	$(CC) $(CFLAGS) server.c common.o queue.o -o server.o -lpthread

client.o: client.c
	$(CC) $(CFLAGS) client.c -o client.o

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c -o queue.o

clean:
	rm -f *.o  *.a a.out
