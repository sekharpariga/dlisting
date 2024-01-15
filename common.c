#include "common.h"

#ifdef HAVE_ST_BIRTHTIME
#define birthtime(x)	x.st_birthtime
#else
#define birthtime(x)	x.st_ctime
#endif

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
	send(*(pclient->client_socket), "#####", strlen("#####"), 0);
}

char * pwdfun(node_t *pclient)
{
	char *old_path = malloc(PATH_MAX * sizeof(char));
	snprintf(old_path, PATH_MAX, "%s", pclient->pwd);
	
	if(getcwd(pclient->pwd, PATH_MAX) == NULL)		//on failure of setting path, restores prev path
	{
		memset(pclient->pwd, 0, PATH_MAX);
		snprintf(pclient->pwd, PATH_MAX, "%s", old_path);
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
	char *buffer = malloc(BUFSIZE * sizeof(char));
	char *tmp = malloc(BUFSIZE * sizeof(char));
	char ending[] = "#####";
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
				filectime = ctime(&birthtime(type));

				if(dir->d_type != DT_REG)
					cpylen = snprintf(tmp, BUFSIZE, "dir\t%s\t%s", dir->d_name, filectime);
				else
					cpylen = snprintf(tmp, BUFSIZE, "file\t%s\t%s", dir->d_name, filectime);

				tmp[cpylen] = 0;

				if((cpylen + msglen) < BUFSIZE)
				{
					strncpy(buffer + msglen, tmp, cpylen);
					msglen += cpylen;
				}
				else
				{
					buffer[msglen] = 0;
					send(clientfd, buffer, msglen, 0);
					memset(buffer, 0, BUFSIZE);
					strncpy(buffer, tmp, cpylen);
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
	}

	if(msglen > 0 && cpylen != 0)
		send(clientfd, buffer, msglen, 0);

	send(clientfd, ending, strlen(ending), 0);	//sending msg ending for client read to close

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
				char *sendbuffer = malloc((len + 6) * sizeof(char));
				strncpy(sendbuffer, pclient->pwd, len);
				strncpy(sendbuffer + len, "#####", 6);
				send(clientfd, sendbuffer, len + 5, 0);
				free(sendbuffer);
			}
			else if(strcmp(task->cmd, "bye") == 0)
			{
				free(buffer);
				free(pclient);
				return 1;
			}
			else
				send(clientfd, "Wrong Request#####", strlen("Wrong Request#####"), 0);
		}
	}
	else
		send(clientfd, "Wrong Request#####", strlen("Wrong Request#####"), 0);
	
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
