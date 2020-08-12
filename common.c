#include "common.h"

extern pthread_mutex_t lock;

void cdfun(char *path, node_t *pclient)
{
	char *cflag = NULL;
	pthread_mutex_lock(&lock);
	int flag = chdir(pclient->pwd);

	if(!flag)
	{
		flag = chdir(path);
		if(!flag)
		{
			cflag = pwdfun(pclient);
			if(cflag != NULL)
				free(cflag);
		}
		else
			printf("Dir not exist, client conn:%d\n", *(pclient->client_socket));
	}
	else
		printf("pwd buffer is wrong:%d\n", *(pclient->client_socket));

	pthread_mutex_unlock(&lock);
	send(*(pclient->client_socket), "0\n", strlen("0\n"), 0);
}

char *pwdfun(node_t *pclient)
{
	char *old_path = malloc(PATHMAX * sizeof(char));
	snprintf(old_path, PATHMAX, "%s", pclient->pwd);
	
	if(getcwd(pclient->pwd, PATHMAX) == NULL)		//on failure of setting path, restores prev path
	{
		memset(pclient->pwd, 0, PATHMAX);
		snprintf(pclient->pwd, PATHMAX, "%s", old_path);
		free(old_path);
		return strdup("E");
	}
	free(old_path);
	return NULL;
}

void lsfun(node_t *pclient)
{
	DIR *directory;
	int status;
	int cpylen = 0;
	int msglen = 0;
	int clientfd = *(pclient->client_socket);
	char *filectime;
	char *buffer = (char *) malloc((BUFSIZE + 5) * sizeof(char));
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
				cpylen = strlen("\t") * 2 + strlen(dir->d_name) + strlen(filectime) ;
				cpylen = dir->d_type == DT_REG ? cpylen + 4: cpylen + 3 ;

				if(dir->d_type != DT_REG)
					snprintf(tmp, BUFSIZE, "dir\t%s\t%s", dir->d_name, filectime);
				else
					snprintf(tmp, BUFSIZE, "file\t%s\t%s", dir->d_name, filectime);

				tmp[cpylen] = 0;

				if((cpylen + msglen) < BUFSIZE)
				{
					strlcpy(buffer + msglen, tmp, cpylen);
					msglen += cpylen;
				}
				else
				{
					snprintf(buffer, BUFSIZE + 5, "%s00000", buffer);
					send(clientfd, buffer, strlen(buffer), 0);
					memset(buffer, 0, BUFSIZE + 5);
					strlcpy(buffer, tmp, cpylen);
					msglen = cpylen;
					cpylen = 0;
				}
			}
			else
			{
				cpylen = snprintf(buffer + msglen, BUFSIZE, "%s%s\n", "file stat error:", dir->d_name);
				msglen += cpylen;
			}
			memset(tmp, 0, BUFSIZE);
		}

		if(msglen != 0)
		{
			snprintf(buffer, BUFSIZE + 5, "%s00000", buffer);
			send(clientfd, buffer, strlen(buffer), 0);
		}
	}
	else
		send(clientfd, "\n00000", strlen("\n00000"), 0);

	free(tmp);
	free(buffer);
	closedir(directory);
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
		buffer = buffertemp;

		if(task->cmd != NULL)
		{
			if(strcmp(task->cmd, "ls") == 0)
				lsfun(pclient);
			else if(strcmp(task->cmd, "cd") == 0 && task->arg != NULL)
				cdfun(task->arg, pclient);
			else if(strcmp(task->cmd, "pwd") == 0)
			{
				int len = strlen(pclient->pwd);
				char *sendbuffer = malloc(len * sizeof(char));
				strlcpy(sendbuffer, pclient->pwd, len);
				printf("len:%d\tbuffer:%s\n", len, sendbuffer);

				snprintf(buffer, BUFSIZE, "0%s\n", sendbuffer);
				send(clientfd, buffer, BUFSIZE, 0);
			}
			else if(strcmp(task->cmd, "bye") == 0)
			{
				free(buffer);
				buffer = NULL;
				free(pclient);
				return 1;
			}
			else
				send(clientfd, "Wrong Request00000\n", strlen("Wrong Request00000\n"), 0);
		}
	}
	else
		send(clientfd, "Wrong Request00000\n", strlen("Wrong Request00000\n"), 0);

	if(buffer != NULL)
		free(buffer);

	return 0;
}

void *threadhandle(__attribute__((unused)) void *arg)
{
	int ret = 0, connfd = 0;
	node_t *pclient;

	do
	{
		pclient = dequeue();
		if(pclient != NULL)
		{	
			connfd = *(pclient->client_socket);
			printf("new conn:%d\n", connfd);

			do
			{
				ret = handleclient(pclient);
			}while(ret == 0);

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
