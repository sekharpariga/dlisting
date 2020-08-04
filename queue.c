#include "queue.h"

node_t *head = NULL;
node_t *tail = NULL;

extern pthread_mutex_t lock;

void enqueue(int *client_socket)
{
	node_t *newnode = malloc(sizeof(node_t));
	pthread_mutex_lock(&lock);
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
	pthread_mutex_unlock(&lock);
}
	
int *dequeue()
{
	if(head == NULL)
		return NULL;
	else
	{
		pthread_mutex_lock(&lock);
		node_t *temp = head;
		int *result = head->client_socket;
		head = head->next;
		if(head == NULL) tail = NULL;
		free(temp);
		pthread_mutex_unlock(&lock);
		return result;
	}
}

void display()
{
	node_t *temp = head;

	while(temp != NULL)
	{
		printf("%d->",*(temp->client_socket));
		temp = temp->next;
	}
	printf("\n");
}
