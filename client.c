#include "common.h"

int main()
{
	int clifd, msgsize;
	char *buffer, *buffertemp, *temp;
	struct sockaddr_in seraddr;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	
	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	seraddr.sin_port = htons(PORT);

	clifd = socket(AF_INET,SOCK_STREAM, 0);
	
	if(connect(clifd, (struct sockaddr *) &seraddr, sizeof(seraddr)) < 0)
		exit(5);

	while(true)
	{
		buffertemp = buffer;
		buffer = buffer + sizeof(int);
		msgsize = read(0, buffer, BUFSIZE - sizeof(int));
		printf("client:str len:%d\tbuffer:%s\n", msgsize, buffer);


		if(msgsize > 0)
		{
			temp = buffer + (msgsize - 1);
	
			while(isspace(*temp))
			{
				temp--;
				msgsize--;
			}

			temp[1] = 0;
			buffer = buffertemp;
			memcpy(buffer, (void *) &msgsize, sizeof(int));
			printf("client:trimed str len:%d-\tbuffer:%s-\n", msgsize, buffer + sizeof(int));

			msgsize = write(clifd, buffer, msgsize*sizeof(char) + sizeof(int));
			msgsize = read(clifd, buffer, BUFSIZE);

			if(msgsize < BUFSIZE)
				buffer[msgsize] = 0;
			else
				buffer[BUFSIZE] = 0;
			printf("server:str len:%d\t:%s\n", msgsize, buffer);
		}

		if(msgsize < 0)
			write(clifd, "bye", sizeof("bye"));

		memset(buffer, 0, BUFSIZE);
	}

	return 0;
}
