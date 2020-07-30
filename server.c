#include "common.h"

int serverfd, clientfd;

char *cdfun(char *path)
{
	int flag = chdir(path);
	if(!flag)
		return strdup("worng dir\n");
	return strdup("dir changed!\n");
}

char *pwdfun()
{
	char *ret = (char*) malloc(BUFSIZE * sizeof(char));
	if(getcwd(ret, BUFSIZE) == NULL)
		ret = strerror(errno);
	return ret;
}

int handleClient(int clientfd)
{
	char *buffer, *buffertemp;
	struct parsedata *task;
	int nbytes, msgsize;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	buffertemp = buffer;

	nbytes = read(clientfd, buffer, BUFSIZE);
	memcpy(&msgsize, buffer, sizeof(int));
	buffer = buffer + sizeof(int);

	if(nbytes > 0) 
	{
		buffer[msgsize] = 0;
		task = clientrequest(buffer, msgsize + 1);
		free(buffertemp);
		buffer = NULL;
		printf("client request::%s-\targ:%s-\n", task->cmd, task->arg);

		if(task->cmd != NULL)
		{
			if(strcmp(task->cmd, "ls") == 0)
				buffer = lsfun();
			else if(strcmp(task->cmd, "cd") == 0 && task->arg != NULL)
				buffer = cdfun(task->arg);
			else if(strcmp(task->cmd, "pwd") == 0)
				buffer = pwdfun();
			else if(strcmp(task->cmd, "byte") == 0)
				buffer = NULL;
		}

		if(buffer != NULL && strlen(buffer) > 0)
			nbytes = send(clientfd, buffer, strlen(buffer), 0);
		else
			nbytes = send(clientfd, "Wrong Input", strlen("Wrong Input"), 0);
	}
	else
		send(clientfd, "Wrong Request", strlen("Wrong Request"), 0);

	free(buffer);
	return nbytes;
}

int main()
{
	struct sockaddr_in address;
	int addrlen = sizeof(address), opt = 1;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	serverfd = socket(AF_INET,SOCK_STREAM, 0);

	if(serverfd == 0)
		exit(1);
	
	if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | TCP_NODELAY, &opt, sizeof(opt)))
	       exit(6);

	if(bind(serverfd,(struct sockaddr *) &address, addrlen) != 0)
		exit(2);

	if(listen(serverfd, MAXCONN) != 0)
		exit(3);

	if((clientfd = accept(serverfd, (struct sockaddr *) &address, (socklen_t *) &addrlen )) <= 0)
		exit(4);
	else
		while(true)
			if(handleClient(clientfd) == 0)
				break;
	return 0;
}
