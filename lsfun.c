#include "common.h"
#define st_time st_ctime.tv_sec
extern pthread_mutex_t lock;
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
