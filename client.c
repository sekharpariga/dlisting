#include "common.h"

int main()
{
	int clifd, nochar;
	char buffer[BUFSIZE];
	struct sockaddr_in seraddr;
	
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(PORT);

	clifd = socket(AF_INET,SOCK_STREAM, 0);
	
	if(connect(clifd, (struct sockaddr *) &seraddr, sizeof(seraddr)) < 0)
		exit(5);
	while(1)
	{
		nochar = read(0,buffer, 4);
		if(nochar > 1)
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
	}

	return 0;
}
