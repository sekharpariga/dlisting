#include "common.h"

pthread_mutex_t lock;

int main()
{
	pthread_t thread_id[THREADPOOL];
	int serverfd, clientfd;
	struct sockaddr_in address;
	int addrlen = sizeof(address), opt = 1;

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
		exit(1);
	}

	serverfd = socket(AF_INET,SOCK_STREAM, 0);

	if(serverfd == 0)
		exit(2);
	
	if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | TCP_NODELAY, &opt, sizeof(opt)))
	       exit(3);

	if(bind(serverfd,(struct sockaddr *) &address, addrlen) != 0)
		exit(4);

	if(listen(serverfd, BACKLOG) != 0)
		exit(5);

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
