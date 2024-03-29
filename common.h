#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include "queue.h"

#define BUFSIZE 1400
#define BACKLOG 50
#define SERVERIP "127.0.0.1"
#define PORT 54321
#define THREADPOOL 10
#define true 1
#define st_time st_ctime.tv_sec

struct parsedata
{
	char *cmd;
	char *arg;
};

void signal_handler(int signum);

void cdfun(char *path, node_t *pclient);

char *pwdfun(node_t *pclient);

int handleclient(node_t *pclient);

void *threadhandle(__attribute__((unused)) void *arg);

void lsfun(node_t *pclient);

struct parsedata * clientrequest(char * data, int msgsize);

#endif
