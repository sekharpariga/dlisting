#include "common.h"

int serfd, clifd;

char * cdfun(char *path)
{
	printf("in cdfun\n");
	int flag = chdir(path);
	printf("%s\n", path);
	if(flag == 0)
		return strdup("cdfun");
	return NULL;
}

char * pwdfun()
{
	char *flag;
	char *ret = (char*) malloc(BUFSIZE * sizeof(char));
	flag = getcwd(ret, (BUFSIZE * sizeof(char) - sizeof(char)));
	if(flag == NULL)
	{
		free(ret);
		ret = strdup("getcwd error");
	}
	flag = NULL;
	return ret;
}


int handleClient(int clifd)
{
	char *buffer, *buffertemp;
	struct parsedata *task;
	int nbytes, msgsize;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	buffertemp = buffer;
	nbytes = read(clifd, buffer, BUFSIZE);
	memcpy(&msgsize, buffer, sizeof(int));
	buffer = buffer + sizeof(int);

	if(nbytes > 0) 
	{
		buffer[msgsize] = 0;
		printf("client:readsize:%d\tmsgsize:%d\tbuffer:%s-\n", nbytes, msgsize, buffer);
		char *fbuffer = (char *) malloc((msgsize + 1) * sizeof(char) );
		strncpy(fbuffer, buffer, msgsize);
		fbuffer[msgsize] = 0;
		task = clientrequest(fbuffer);
		free(buffertemp);
		buffer = NULL;
		printf("client:cmd:%s-\targ:%s-\n", task->cmd, task->arg);

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

			if(buffer != NULL)
				printf("client:fun:buffer:%s-\n", buffer);
		}

		if(buffer != NULL && strlen(buffer) > 0)
			nbytes = send(clifd, buffer, strlen(buffer), 0);
		else
			nbytes = send(clifd, "server:wrong inp\n", strlen("server:wrong inp\n"), 0);
	
		free(buffer);
	}
	else
	{
		free(buffer);
		send(clifd, "server:no request\n", strlen("server:no request\n"), 0);
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
	
	if(setsockopt(serfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | TCP_NODELAY, &opt, sizeof(opt)))
	       exit(6);

	if(bind(serfd,(struct sockaddr *) &address, addrlen) != 0)
		exit(2);

	if(listen(serfd, MAXCONN) != 0)
		exit(3);

	if((clifd = accept(serfd, (struct sockaddr *) &address, (socklen_t *) &addrlen )) <= 0)
		exit(4);
	else
		while(true)
			if(handleClient(clifd) == 0)
				break;
	return 0;
}
