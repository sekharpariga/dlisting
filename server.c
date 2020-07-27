#include "common.h"

int serfd, clifd;

char * byefun()
{
	char *ret = (char*) malloc(BUFSIZE * sizeof(char));
	strncpy(ret, "bye", 4);
	return ret;
}

char * cdfun(char *path)
{
	char *ret = (char*) malloc(BUFSIZE * sizeof(char));
	printf("%s\n", path);
	strncpy(ret, "cdfun", 6);
	return ret;
}

char * pwdfun()
{

	char *ret = (char*) malloc(BUFSIZE * sizeof(char));
	strncpy(ret, "pwdfun", 7);
	return ret;
}


int handleClient(int clifd)
{
	char *buffer;
	struct parsedata *task;
	int nbytes = 0;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	nbytes = read(clifd, buffer, BUFSIZE);
	printf("len:%d\n", nbytes);

	if(nbytes > 0) 
	{
		buffer[nbytes-1] = 0;
		task = clientrequest(buffer);
		free(buffer);
		buffer = NULL;

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
			else
				buffer = NULL;
			printf("buffer --->\n%s\n", buffer);

		}

		if(buffer != NULL && strlen(buffer) > 0)
			nbytes = send(clifd, buffer, strlen(buffer), 0);
		else
			nbytes = send(clifd, "bye", strlen("bye"), 0);
	
		free(buffer);
	}
	else
	{
		free(buffer);
		send(clifd, "no request", strlen("no request"), 0);
		nbytes = 0;
	}
	return nbytes;
}

int main()
{
	struct sockaddr_in address;
	int addrlen = sizeof(address), opt = 1;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	serfd = socket(AF_INET,SOCK_STREAM, 0);

	if(serfd == 0)
		exit(1);
	
	if(setsockopt(serfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	       exit(6);

	if(bind(serfd,(struct sockaddr *) &address, addrlen) != 0)
		exit(2);

	if(listen(serfd, 100000) != 0)
		exit(3);

	if((clifd = accept(serfd, (struct sockaddr *) &address, (socklen_t *) &addrlen )) <= 0)
		exit(4);
	else
		while(true)
			if(handleClient(clifd) == 0)
				break;
	return 0;
}
