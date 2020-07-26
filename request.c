#include "request.h"

struct parsedata * clientrequest(char data[] )
{
	struct parsedata *pdata;
	int len = 0;
	printf("yes\n");
	char *token = strtok(data, " ");

	if(token != NULL)
	{
		len = strlen(token);
		printf("1:len:%d", len);
		*pdata->cmd = (char *) malloc((len+1) * sizeof(char));
		strncpy(*pdata->cmd, token, len);
		printf("debug:cmd:%s\n", pdata->cmd);
	}

	token = strtok(NULL, " ");
	
	if(token != NULL)
	{
		len = strlen(token);
		printf("2:len:%d", len);
		*pdata->arg = (char *) malloc((len+1) * sizeof(char));
		strncpy(*pdata->arg, token, len);
		printf("debug:arg:%s\n", pdata->arg);
	}
	
	return pdata;
}

void disrequest(struct parsedata *data)
{
	printf("%s-->%s\n", data->cmd, data->arg);
}

int main()
{
	char arry[] = "Sekhar Pariga";
	disrequest(clientrequest(arry));

	char arry2[] = "b a";
	disrequest(clientrequest(arry2));

	return 0;
}
