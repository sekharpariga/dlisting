#ifndef __QUEUE__H
#define __QUEUE__H

#include "common.h"

typedef struct node 
{
	int *client_socket;
	struct node *next;
} node_t;

#endif
