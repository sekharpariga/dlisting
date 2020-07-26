#include "common.h"
#include "lsfun.h"

int serfd, clifd;
char * cdfun()
{
	char *ret = (char*) malloc(BUFSIZE * sizeof(char));
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
	char cmd[CMDBUFSIZE];
	char *buffer;
	int nbytes;
	nbytes = read(clifd, cmd, 2);
	printf("lenth:%d\t%s\n",nbytes, cmd);
	cmd[2] = 0;

	if(nbytes > 0) 
	{
		cmd[nbytes] = 0;
		switch(nbytes)
		{
			case 2:
				if(strcmp(cmd,"ls") == 0)	buffer = lsfun();
				if(strcmp(cmd, "cd") == 0)	buffer = cdfun();
				break;
			case 3:
				if(strcmp(cmd, "pwd") == 0)	buffer = pwdfun();
				break;
			case 4:
				if(strcmp(cmd, "bye") == 0)	nbytes = 0;
				break;
			default:
				printf("\ncmd : %s\n", cmd);
				nbytes = 0;
				break;
		}
	}
	if(strlen(buffer))
		nbytes = send(clifd, buffer, strlen(buffer), 0);
	printf("length:%ld\tclient fd:%d\n%s\n", strlen(buffer), clifd, buffer);
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

	//while(1)
	{
		if((clifd = accept(serfd, (struct sockaddr *) &address, (socklen_t *) &addrlen )) <= 0)
			exit(4);
		else
			while(1){
				int val = handleClient(clifd);
				if(val == 0)
					break;
			}
	}

	return 0;
}
