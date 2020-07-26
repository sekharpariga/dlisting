#ifndef __REQUEST__H
#define __REQUEST__H

#include "common.h"

struct parsedata
{
	char *cmd;
	char *arg;
};

struct parsedata * clientrequest(char * data);

#endif
