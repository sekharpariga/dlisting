#include "common.h"
#define st_time st_ctime.tv_sec

char *lsfun()
{
	DIR *directory;
	int status, len, cpylen = 0;
	char *tmprec, *filectime;
	struct dirent *dir;
	struct stat type;

	directory = opendir(".");
	char *tmp = (char *) malloc(BUFSIZE * sizeof(char));
	char *ret = (char *) malloc(BUFSIZE * sizeof(char));
	int msglen = 0;

	if(tmp == NULL && ret == NULL)
		perror("malloc error\n");

	if(directory)
	{
		while((dir = readdir(directory)) != NULL)
		{
			if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
				continue;
			status = stat(dir->d_name, &type);

			if(status == 0)
			{
				filectime = ctime(&type.st_ctime);		//st_birthtime is not available
				len = sizeof("\t") * 2 + strlen(dir->d_name) + strlen(filectime) + 1;
				len = dir->d_type == DT_REG ? len + 4: len + 3 ; 	//4 ---> strlen("file"), 3 ---> strlen("dir")

				if(dir->d_type != DT_REG)
					snprintf(tmp, BUFSIZE, "dir\t%s\t%s", dir->d_name, filectime);
				else
					snprintf(tmp, BUFSIZE, "file\t%s\t%s", dir->d_name, filectime);

				tmp[len] = 0;
				cpylen = strlen(tmp);
				cpylen = (msglen + cpylen) < BUFSIZE ? cpylen : (BUFSIZE - msglen - 1);
				if(cpylen > 0)
				{
					strncpy(ret + msglen, tmp, cpylen);
					msglen += cpylen;
				}
				else
					break;
			}
		}
	}

	if(msglen < BUFSIZE)
		ret[msglen] = 0;
	else
		ret[BUFSIZE] = 0;

	printf("full lenth:%d\n", msglen);
	free(tmp);
	return ret;
}
