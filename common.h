#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
#define MAXCONN 1000
#define SERVERIP "127.0.0.1"
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
