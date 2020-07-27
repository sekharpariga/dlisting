#include "common.h"

int main()
{
	int clifd, nochar;
	char *buffer, *temp;
	struct sockaddr_in seraddr;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(PORT);

	clifd = socket(AF_INET,SOCK_STREAM, 0);
	
	if(connect(clifd, (struct sockaddr *) &seraddr, sizeof(seraddr)) < 0)
		exit(5);

	while(true)
	{
		nochar = read(0, buffer, BUFSIZE);
		printf("first len :%d\n", nochar);
		while(isspace( *buffer))
		{
			buffer++;
			nochar--;
		}

		temp = buffer + nochar;

		while(isspace(*temp))
		{
			temp--;
			nochar--;
		}

		temp[1] = 0;
		temp = NULL;

		printf("after trim len :%d\n", nochar);
		printf("-%s-\n", buffer);
		if(nochar > 0)
		{
			nochar = write(clifd,buffer,nochar);
			nochar = read(clifd, buffer, BUFSIZE);

			if(nochar < BUFSIZE)
				buffer[nochar] = 0;
			else
				buffer[BUFSIZE] = 0;
			printf("server message received\n%s\n", buffer);
		}

		if(nochar < 0)
			write(clifd, "bye", sizeof("bye"));
		buffer[0] = 0;
	}

	return 0;
}
