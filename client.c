#include "common.h"

int socketfd;
char *buffer = NULL;

void sigexit()
{
	int numbytes = 3;
	memcpy(buffer, &numbytes, sizeof(int));

	snprintf(buffer + sizeof(int), BUFSIZE, "bye");
	write(socketfd, buffer, 3*sizeof(char) + sizeof(int));

	close(socketfd);
	exit(0);
}

void signal_handler_client(int num)
{

	if(num == SIGTSTP)
		sigexit();
	if(num == SIGINT)
		sigexit();
}

int main()
{
	int msgsize;
	char *buffertemp, *temp, dataflag;
	struct sockaddr_in seraddr;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	
	seraddr.sin_family = AF_INET;
	seraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	seraddr.sin_port = htons(PORT);

	socketfd = socket(AF_INET,SOCK_STREAM, 0);

	if(connect(socketfd, (struct sockaddr *) &seraddr, sizeof(seraddr)) < 0)
		exit(11);

	if(signal(SIGINT, signal_handler_client) == SIG_ERR)
		perror("error in SIGTERM registering\n");

	if(signal(SIGTSTP, signal_handler_client) == SIG_ERR)
		perror("error in SIGTSTP registering\n");

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
				close(socketfd);
				free(buffer);
				exit(0);
			}
			memset(buffer, 0, BUFSIZE);

			do
			{
				msgsize = read(socketfd, buffer, BUFSIZE);
				dataflag = buffer[0];
				printf("\ndataflag:%c\n", dataflag);
				if(strlen(buffer) > 0 && strlen(buffer) < BUFSIZE)
				buffer[strlen(buffer)] = 0;
				
				printf("%s", buffer + 1);
				memset(buffer, 0, BUFSIZE);
			}while(dataflag != '0');
		}
		memset(buffer, 0, BUFSIZE);
	}
	free(buffer);
	close(socketfd);
	return 0;
}
