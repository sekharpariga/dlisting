#ifndef __QUEUE__H
#define __QUEUE__H

#include "common.h"

typedef struct node 
{
	int *client_socket;
	char pwd[PATH_MAX];
	struct node *next;
} node_t;

void enqueue(int *client_socket, char *pwd);
node_t *dequeue();
void display();

#endif
