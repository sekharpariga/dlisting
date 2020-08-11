#ifndef __QUEUE__H
#define __QUEUE__H

#define PATHMAX 1400
typedef struct node 
{
	int *client_socket;
	char pwd[PATHMAX];
	struct node *next;
} node_t;

void enqueue(int *client_socket, char *pwd);
node_t *dequeue();
void display();

#endif
