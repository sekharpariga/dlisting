#include "common.h"

int serverfd, clientfd;
struct sockaddr_in address;
int addrlen = sizeof(address), opt = 1, ret = 0;
pthread_t thread_id[THREADPOOL];
pthread_mutex_t lock;

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

int handleclient(int *client_socket)
{
	char *buffer, *buffertemp;
	struct parsedata *task;
	int msgsize = 0, clientfd = *client_socket;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	buffertemp = buffer;

	read(clientfd, buffer, BUFSIZE);
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
				return -1;
			else
				buffer = NULL;
		}

		if(buffer != NULL && strlen(buffer) > 0)
			send(clientfd, buffer, strlen(buffer), 0);
		else
			send(clientfd, "Wrong Request", strlen("Wrong Request"), 0);

	}
	else
		send(clientfd, "Wrong Request", strlen("Wrong Request"), 0);

	if(buffer != NULL)
		free(buffer);

	return msgsize;
}

void *threadhandle(void *arg)
{
	int *pclient;
	while(true)
	{
		pthread_mutex_lock(&lock);
		pclient = dequeue();
		pthread_mutex_unlock(&lock);
		if(pclient != NULL)
			while(handleclient(pclient) == -1)
				break;
	}
	return NULL;
}

int main()
{

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	int *pclient;
	
	if(pthread_mutex_init(&lock, NULL) != 0)
	{
		perror("mutex lock init failed!\n");
		exit(-1);
	}

	serverfd = socket(AF_INET,SOCK_STREAM, 0);

	if(serverfd == 0)
		exit(1);
	
	if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | TCP_NODELAY, &opt, sizeof(opt)))
	       exit(6);

	if(bind(serverfd,(struct sockaddr *) &address, addrlen) != 0)
		exit(2);

	if(listen(serverfd, BACKLOG) != 0)
		exit(3);

	for(int i = 0; i < THREADPOOL; i++)
	{
		pthread_create(&(thread_id[i]), NULL, &threadhandle, NULL);
		pthread_join(thread_id[i], NULL);
	}

	while(true)
	{
		if ((clientfd = accept(serverfd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0)
		{
			printf("new conn:%d\n", clientfd);
			pclient = (int *) malloc(sizeof(int));
			if(pclient == NULL)
				perror("malloc error\n");
			else
			{
				pthread_mutex_lock(&lock);
				enqueue(pclient);
				pthread_mutex_lock(&lock);
			}
		}
	}

	close(serverfd);
	return 0;
}
