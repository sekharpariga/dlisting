#include "queue.h"

struct node 
{
	int *clifd;
	struct node *next;
};

typedef node node_t;
node_t *queue;

void enqueue(int *val);
void dequeue(int *ptr)
{
	while(1)
}
