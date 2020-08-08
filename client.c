#include "common.h"

int main()
{
	int socketfd, msgsize;
	char *buffer, *buffertemp, *temp;
	struct sockaddr_in seraddr;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	
	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	seraddr.sin_port = htons(PORT);

	socketfd = socket(AF_INET,SOCK_STREAM, 0);
	
	if(connect(socketfd, (struct sockaddr *) &seraddr, sizeof(seraddr)) < 0)
		exit(5);

	while(true)
	{
		buffertemp = buffer;
		buffer = buffer + sizeof(int);
		msgsize = read(0, buffer, BUFSIZE - sizeof(int));

		if(msgsize > 1)
		{
			temp = buffer + (msgsize - 1);
	
			while(isspace(*temp))
			{
				temp--;
				msgsize--;
			}

			temp[1] = 0;
			buffer = buffertemp;
			memcpy(buffer, &msgsize, sizeof(int));

			write(socketfd, buffer, msgsize*sizeof(char) + sizeof(int));

			if(strncmp(buffer + sizeof(int), "bye", sizeof("bye")) == 0)
			{
				exit(0);
				free(buffer);
			}
			
			do{
				msgsize = read(socketfd, buffer, BUFSIZE);
				if(msgsize < BUFSIZE)
					buffer[msgsize] = 0;
				else
					buffer[BUFSIZE] = 0;

				printf("%s", buffer + 1);
			}while(*buffer != '0');
		}
		memset(buffer, 0, BUFSIZE);
	}
	free(buffer);
	close(socketfd);
	return 0;
}
