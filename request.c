#include "request.h"

struct parsedata * clientrequest_old(char *data)
{
	struct parsedata *pdata = (struct parsedata *) malloc(sizeof(struct parsedata));
	int len = 0;
	char *token = strtok(data, " ");

	if(token != NULL)
	{
		len = strlen(token);
		pdata->cmd = (char *) malloc((len+1) * sizeof(char));
		strncpy( pdata->cmd, token, len);
	}
	else
		pdata->cmd = NULL;

	token = strtok(NULL, " ");
	
	if(token != NULL)
	{
		len = strlen(token);
		pdata->arg = (char *) malloc((len+1) * sizeof(char));
		strncpy(pdata->arg, token, len);
	}
	else
		pdata->arg = NULL;
	
	return pdata;
}

struct parsedata * clientrequest(char *data, int msgsize)
{
	int cmdstart = 0, argstart = 0, cmdlen = 0, arglen = 0, index = 0;
	char *temp = data, *cmd = NULL, *arg = NULL;
	struct parsedata *pdata = (struct parsedata *) malloc(sizeof(struct parsedata));

	while(*data == ' '  && index < msgsize) 
	{
		cmdstart++;
		data++;
		index++;
	}

	while(*data != ' ' && index < msgsize)
	{
		cmdlen++;
		data++;
		index++;
	}
	
	argstart = cmdstart + cmdlen;
	while(*data == ' ' && index < msgsize)
	{
		argstart++;
		data++;
		index++;
	}

	while(*data != ' ' && index < msgsize)
	{
		arglen++;
		data++;
		index++;
	}

	if(cmdlen > 0 && cmdlen < msgsize)
		cmd = (char *) malloc(sizeof(char) * (cmdlen + 1));
	if(arglen > 0 && arglen < msgsize)
		arg = (char *) malloc(sizeof(char) * (arglen + 1));

	strncpy(cmd, temp + cmdstart, cmdlen);
	cmd[cmdlen] = 0;
	strncpy(arg, temp + argstart, arglen);
	arg[arglen] = 0;

	pdata->cmd = cmd;
	pdata->arg = arg;

	return pdata;
}
