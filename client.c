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
	char *buffertemp, *temp;
	char ending[10];
	struct sockaddr_in seraddr;
	buffer = malloc(BUFSIZE * sizeof(char));
	
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
				close(socketfd);
				return 0;
			}
			memset(buffer, 0, BUFSIZE);

			while(true)
			{
				msgsize = read(socketfd, buffer, BUFSIZE);
				buffer[msgsize] = 0;
				snprintf(ending, 6, "%s", buffer + (strlen(buffer) - 5));

				if(strcmp(ending, "#####") == 0)
				{
					printf("msgsize:%d\n", msgsize);
					buffer[msgsize - 6] = 0;

					if(buffer[msgsize - 6] == '\n')
						printf("%s", buffer);
					else
						printf("%s\n", buffer);
					fflush(stdout);
					break;
				}
				else
				{
					printf("%s", buffer);
					fflush(stdout);
					memset(buffer, 0, BUFSIZE);
				}
			}
		}
		memset(buffer, 0, BUFSIZE);
	}

	close(socketfd);
	return 0;
}
