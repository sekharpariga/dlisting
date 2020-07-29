#include "request.h"

struct parsedata * clientrequest(char *data)
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
