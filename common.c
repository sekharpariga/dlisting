#include "common.h"

extern pthread_mutex_t lock;

char *cdfun(char *path, node_t *pclient)
{
	char *cflag = NULL;
	char *currentpath = strdup(pclient->pwd);
	pthread_mutex_lock(&lock);
	int flag = chdir(pclient->pwd);

	if(!flag)
	{
		flag = chdir(path);
		if(!flag)
		{
			cflag = pwdfun(pclient);
			if(cflag != NULL)
				strlcpy(pclient->pwd, currentpath, PATH_MAX);
		}
		else
			printf("Wrong Dir Requested by client conn:%d\n", *(pclient->client_socket));
	}
	else
		printf("Wrong pclient->pwd, client conn:%d", *(pclient->client_socket));
	pthread_mutex_unlock(&lock);
	return strdup(" ");
}

char *pwdfun(node_t *pclient)
{
	if(getcwd(pclient->pwd, PATH_MAX) == NULL)
		strdup("error in asigning path");
	return NULL;
}

char *lsfun(node_t *pclient)
{
	DIR *directory;
	int status;
	int recordlen;
	int cpylen = 0;
	int msglen = 0;
	int clientfd = *(pclient->client_socket);
	char *filectime;
	char *buffer = (char *) malloc(BUFSIZE * sizeof(char));
	char *tmp = (char *) malloc(BUFSIZE * sizeof(char));
	struct dirent *dir;
	struct stat type;

	pthread_mutex_lock(&lock);
	chdir(pclient->pwd);
	directory = opendir(".");
	pthread_mutex_unlock(&lock);

	if(tmp == NULL || buffer == NULL)
		printf("malloc error for client conn:%d\n", clientfd);

	if(directory)
	{
		while((dir = readdir(directory)) != NULL)
		{
			if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
				continue;
			status = stat(dir->d_name, &type);

			if(status == 0)
			{
				filectime = ctime(&type.st_ctime);
				recordlen = sizeof("\t") * 3 + strlen(dir->d_name) + strlen(filectime) + 1;
				recordlen = dir->d_type == DT_REG ? recordlen + 4: recordlen + 3 ;
				
				if(dir->d_type != DT_REG)
					snprintf(tmp, BUFSIZE, "dir\t%s\t\t%s", dir->d_name, filectime);
				else
					snprintf(tmp, BUFSIZE, "file\t%s\t\t%s", dir->d_name, filectime);

				tmp[recordlen] = 0;
				cpylen = strlen(tmp);

				if((cpylen + msglen) < BUFSIZE)
				{
					printf("1:cpylen:%d, msglen:%d\n", cpylen, msglen);
					strlcpy(buffer + msglen, tmp, cpylen);
					msglen += cpylen;
				}
				else if((cpylen + msglen) >= BUFSIZE)
				{
					printf("2:cpylen:%d, msglen:%d, sum :%d\n", cpylen, msglen, (msglen+cpylen));
					printf("buffer:%s\n", buffer);
					send(clientfd, buffer, strlen(buffer), 0);
					strlcpy(buffer, tmp, cpylen);
					msglen = cpylen;
				}
				else
					printf("%s\n", buffer);
			}
			else
				send(clientfd, "error in lsfun",strlen("error in lsfun"), 0);
			cpylen = 0;
		}
	}
	
	printf("3: cpylen:%d, msglen:%d\n", cpylen, msglen);
	if(msglen != 0)
		send(clientfd, buffer, strlen(buffer), 0);

	free(tmp);
	free(buffer);
	closedir(directory);
	return NULL;
}

int handleclient(node_t *pclient)
{
	char *buffer, *buffertemp;
	struct parsedata *task;
	int msgsize = 0, clientfd = *(pclient->client_socket);
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	buffertemp = buffer;

	read(clientfd, buffer, BUFSIZE);
	memcpy(&msgsize, buffer, sizeof(int));
	buffer = buffer + sizeof(int);
	
	if(msgsize > 1) 
	{
		buffer[msgsize] = 0;
		task = clientrequest(buffer, msgsize + 1);
		free(buffertemp);

		if(task->cmd != NULL)
		{
			if(strcmp(task->cmd, "ls") == 0)
				buffer = lsfun(pclient);
			else if(strcmp(task->cmd, "cd") == 0 && task->arg != NULL)
				buffer = cdfun(task->arg, pclient);
			else if(strcmp(task->cmd, "pwd") == 0)
				buffer = pclient->pwd;
			else if(strcmp(task->cmd, "bye") == 0)
			{
				free(pclient);
				return -1;
			}
			else
				buffer = NULL;
		}
		if(buffer != NULL)
			send(clientfd, buffer, strlen(buffer), 0);
	}
	else
		send(clientfd, "Wrong Request", strlen("Wrong Request"), 0);

	return msgsize;
}

void *threadhandle(__attribute__((unused)) void *arg)
{
	int ret = 0, connfd = 0;
	node_t *pclient;
	do{
		pclient = dequeue();
		if(pclient != NULL)
		{	
			connfd = *(pclient->client_socket);
			printf("new conn:%d\n", connfd);
			do{
				ret = handleclient(pclient);
			}while(ret != -1);
			printf("closed conn:%d\n", connfd);
		}
		else
			usleep(100);

	}while(true);
	return NULL;
}

struct parsedata *clientrequest(char *data, int msgsize)
{
	int cmdstart = 0, argstart = 0, cmdlen = 0, arglen = 0, index = 0;
	char *temp = data, *cmd = NULL, *arg = NULL;
	struct parsedata *pdata = (struct parsedata *) malloc(sizeof(struct parsedata));

	while(*data == ' '  && index < msgsize) 
	{
		cmdstart++;
		data++;
		index++;
	}

	while(*data != ' ' && index < msgsize)
	{
		cmdlen++;
		data++;
		index++;
	}
	
	argstart = cmdstart + cmdlen;

	while(*data == ' ' && index < msgsize)
	{
		argstart++;
		data++;
		index++;
	}

	while(*data != ' ' && index < msgsize)
	{
		arglen++;
		data++;
		index++;
	}

	if(cmdlen > 0)
		cmd = strndup(temp + cmdstart, cmdlen);
	if(arglen > 0)
		arg = strndup(temp + argstart, arglen);

	if(cmdstart == argstart && cmdlen > 0 && arglen == 0)
		arg = NULL;

	pdata->cmd = cmd;
	pdata->arg = arg;

	return pdata;
}

void signal_handler(int signum)
{
	if(signum == SIGINT)
		printf("\nreceived SIGINT\nServer Exiting\n");

	if(signum == SIGTSTP)
		printf("\nreceived SIGSTP\nServer Exiting\n");

	exit(0);
}

