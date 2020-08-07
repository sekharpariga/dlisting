#include "common.h"

extern pthread_mutex_t lock;

void signal_handler(int signum)
{
	if(signum == SIGINT)
	{
		printf("\nreceived SIGINT\nServer Exiting\n");
		usleep(1000);
		exit(0);
	}

	if(signum == SIGTSTP)
	{
		printf("\nreceived SIGSTP\nServer Exiting\n");
		usleep(1000);
		exit(0);
	}
}

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
			strdup("Wrong Dir");
	}
	else
		strdup("Wrong dir pclient->pwd");
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
	int status, len, cpylen = 0, space = 0;
	char *filectime;
	struct dirent *dir;
	struct stat type;
	pthread_mutex_lock(&lock);
	chdir(pclient->pwd);
	directory = opendir(".");
	pthread_mutex_unlock(&lock);
	char *tmp = (char *) malloc(BUFSIZE * sizeof(char));
	char *ret = (char *) malloc(BUFSIZE * sizeof(char));
	int msglen = 0;

	if(tmp == NULL || ret == NULL)
		perror("malloc buffer allocation error\n");

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
				len = sizeof("\t") * 3 + strlen(dir->d_name) + strlen(filectime) + 1;
				len = dir->d_type == DT_REG ? len + 4: len + 3 ;
				
				if(dir->d_type != DT_REG)
					snprintf(tmp, BUFSIZE, "dir\t%s\t\t%s", dir->d_name, filectime);
				else
					snprintf(tmp, BUFSIZE, "file\t%s\t\t%s", dir->d_name, filectime);

				tmp[len] = 0;
				cpylen = strlen(tmp);
				cpylen = (msglen + cpylen) < BUFSIZE ? cpylen : (BUFSIZE - msglen - 1);

				if(cpylen > 0)
				{
					strlcpy(ret + msglen, tmp, cpylen);
					msglen += cpylen;
				}
				else
					break;
			}
			else
				strcpy(ret, "error in lsfun");
		}
	}

	if(msglen < BUFSIZE)
		ret[msglen] = 0;
	else
		ret[BUFSIZE] = 0;

	free(tmp);
	return ret;
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

		if(buffer != NULL && strlen(buffer) > 0)
			send(clientfd, buffer, strlen(buffer), 0);
		else
			send(clientfd, "Invalid command!", strlen("Invalid command!"), 0);
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
			usleep(10);

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
