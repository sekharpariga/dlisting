// A C/C++ program for splitting a string 
// using strtok() 
#include <stdio.h> 
#include "common.h"

void signal_handler(int signum)
{
	if(SIGSTOP == signum)
		printf("caught singstop\n");
}

int main() 
{ 

	if(signal(SIGSTOP, signal_handler) == SIG_ERR)
		perror("unable to caught SIGSTOP\n");

	printf("SIGTERM:%d\n", SIGTERM);
	printf("SIGSTOP:%d\n", SIGSTOP);
	printf("SIGKILL:%d\n", SIGKILL);
	return 0; 
} 

