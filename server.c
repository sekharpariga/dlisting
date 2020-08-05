#include "common.h"

int serverfd, clientfd;
struct sockaddr_in address;
int addrlen = sizeof(address), opt = 1;
pthread_t thread_id[THREADPOOL];
pthread_mutex_t lock;

void signal_handler(int signum)
{
	if(signum == SIGINT)
		printf("received SIGINT\n");

	if(signum == SIGTSTP)
		printf("received SIGTSTP\n");
}

char *cdfun(char *path)
{
	int flag = chdir(path);
	if(flag)
		return strdup("worng dir\n");
	return strdup(" ");
}

char *pwdfun(node_t *pclient)
{
	if(pclient->pwd == NULL)
		return strdup("malloc error\n");

	if(getcwd(pclient->pwd, PATH_MAX) == NULL)
		ret = strerror(errno);
	
	if(strlen(ret) < PATH_MAX-1)
	{
		ret[strlen(ret)] = '\n';
		ret[strlen(ret) + 1] = 0;
	}
	return ret;
}

int handleclient(node_t *pclient)
{
	char *buffer, *buffertemp;
	struct parsedata *task;
	int msgsize = 0, clientfd = *client_socket;
	buffer = (char *) malloc(BUFSIZE * sizeof(char));
	buffertemp = buffer;

	read(pclient->client_socket, buffer, BUFSIZE);
	memcpy(&msgsize, buffer, sizeof(int));
	buffer = buffer + sizeof(int);

	if(signal(SIGINT, signal_handler) == SIG_ERR)		//ctrl-c
		perror("can't catch SIGTERM\n");

	if(signal(SIGTSTP, signal_handler) == SIG_ERR)		//ctrl-z
		perror("can't catch SIGTSTP\n");

	if(msgsize > 0) 
	{
		buffer[msgsize] = 0;
		task = clientrequest(buffer, msgsize + 1);
		free(buffertemp);

		if(task->cmd != NULL)
		{
			if(strcmp(task->cmd, "ls") == 0)
				buffer = lsfun(pclient->pwd);
			else if(strcmp(task->cmd, "cd") == 0 && task->arg != NULL)
				buffer = cdfun(task->arg, pclient->pwd);
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
			send(clientfd, "Wrong Request", strlen("Wrong Request"), 0);

	}
	else
		send(clientfd, "Wrong Request", strlen("Wrong Request"), 0);

	if(buffer != NULL)
		free(buffer);
	return msgsize;
}

void *threadhandle(__attribute__((unused)) void *arg)
{
	int ret = 0;
	node_t *pclient;
	do{
		pclient = dequeue();

		if(pclient != NULL)
		{
			do{
				ret = handleclient(pclient);
			}while(ret > 0);
		}
		else
			usleep(10);

	}while(true);

	printf("thread closed\n");
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
		pthread_create(&(thread_id[i]), NULL, &threadhandle, NULL);

	while(true)
	{
		if ((clientfd = accept(serverfd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) > 0)
		{

			pclient = (int *) malloc(sizeof(int));
			if(pclient == NULL)
				perror("malloc error\n");
			else
			{
				*pclient = clientfd;
				enqueue(pclient);
			}
		}
	}

	for(int i = 0; i < THREADPOOL; i++)
		pthread_join(thread_id[i], NULL);
	
	close(serverfd);
	return 0;
}
