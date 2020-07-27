#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include "lsfun.h"
#include "request.h"

#define BUFSIZE 4096
#define CMDBUFSIZE 5
#define PORT 54321
#define true 1

char *pwdfun();
char *cdfun(char *);
char *byefun();
char *lsfun();

struct msg
{
	char *message;
	int len;
};

#endif
