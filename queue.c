#include "queue.h"

node_t *head = NULL;
node_t *tail = NULL;

extern pthread_mutex_t lock;

void enqueue(int *client_socket, char *path)
{
	node_t *newnode = malloc(sizeof(node_t));

	if(newnode != NULL)
	{
		newnode->client_socket = client_socket;
		newnode->next = NULL;
		strlcpy(newnode->pwd, path, PATH_MAX);

		pthread_mutex_lock(&lock);
		if(tail == NULL)
			head = newnode;
		else
			tail->next =  newnode;
		tail = newnode;
		pthread_mutex_unlock(&lock);
	}
}
	
node_t *dequeue()
{
	if(head == NULL)
		return NULL;
	else
	{
		pthread_mutex_lock(&lock);
		node_t *result = head;
		head = head->next;
		if(head == NULL) tail = NULL;
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
