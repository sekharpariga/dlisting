#include "queue.h"

node_t *head = NULL;
node_t *tail = NULL;

void enqueue(int *client_socket)
{
	node_t *newnode = malloc(sizeof(node_t));
	if(newnode != NULL)
	{
		newnode->client_socket = client_socket;
		newnode->next = NULL;
		if(tail == NULL)
			head = newnode;
		else
			tail->next =  newnode;
		tail = newnode;
	}
}
	
int *dequeue()
{
	if(head == NULL)
		return NULL;
	else
	{
		node_t *temp = head;
		int *result = head->client_socket;
		head = head->next;
		if(head == NULL) tail = NULL;
		free(temp);
		return result;
	}
}
