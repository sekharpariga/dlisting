#include "common.h"

int serverfd, clientfd;
struct sockaddr_in address;
int addrlen = sizeof(address), opt = 1;

pthread_t thread_id[THREADPOOL];
pthread_mutex_t lock;

char *pwdfun(node_t *pclient);

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

int main()
{

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	int *pclient;
	char *mainpwd = malloc(sizeof(char) * PATH_MAX);
	if(getcwd(mainpwd, PATH_MAX) == NULL)
		exit(8);

	if(signal(SIGINT, signal_handler) == SIG_ERR)		//ctrl-c
		perror("can't catch SIGTERM\n");

	if(signal(SIGTSTP, signal_handler) == SIG_ERR)		//ctrl-z
		perror("can't catch SIGTSTP\n");
	
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
				enqueue(pclient, mainpwd);
			}
		}
	}

	for(int i = 0; i < THREADPOOL; i++)
		pthread_join(thread_id[i], NULL);
	
	close(serverfd);
	return 0;
}
