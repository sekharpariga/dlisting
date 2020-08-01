#include "common.h"

int serverfd, clientfd;

char *cdfun(char *path)
{
	int flag = chdir(path);
	if(flag)
		return strdup("worng dir\n");
	return strdup(" ");
}

char *pwdfun()
{
	char *ret = (char*) malloc(BUFSIZE * sizeof(char));
	if(ret == NULL)
		return strdup("malloc error\n");
	if(getcwd(ret, BUFSIZE) == NULL)
	{
		free(ret);
		ret = strerror(errno);
	}
	if(strlen(ret) < BUFSIZE-1)
	{
		ret[strlen(ret)] = '\n';
		ret[strlen(ret) +1] = 0;
	}
	return ret;
}

int handleClient(int clientfd)
{
	char *buffer, *buffertemp;
	struct parsedata *task;
	int nbytes = 0, msgsize = 0;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	buffertemp = buffer;

	nbytes = read(clientfd, buffer, BUFSIZE);
	memcpy(&msgsize, buffer, sizeof(int));
	buffer = buffer + sizeof(int);

	if(msgsize > 0) 
	{
		buffer[msgsize] = 0;
		task = clientrequest(buffer, msgsize + 1);
		free(buffertemp);

		if(task->cmd != NULL)
		{
			if(strcmp(task->cmd, "ls") == 0)
				buffer = lsfun();
			else if(strcmp(task->cmd, "cd") == 0 && task->arg != NULL)
				buffer = cdfun(task->arg);
			else if(strcmp(task->cmd, "pwd") == 0)
				buffer = pwdfun();
			else if(strcmp(task->cmd, "bye") == 0)
			{
				free(buffer);
				return -1;
			}
			else
			{
				free(buffer);
				return 0;
			}
		}

		if(buffer != NULL && strlen(buffer) > 0)
			nbytes = send(clientfd, buffer, strlen(buffer), 0);
	}
	else
		send(clientfd, "Wrong Request", strlen("Wrong Request"), 0);
	if(buffer != NULL)
		free(buffer);
	return 0;
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

	if((clientfd = accept(serverfd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) <= 0)
		exit(4);
	else
		handleClient(clientfd)
	close(serverfd);
	close(clientfd);
	return 0;
}
