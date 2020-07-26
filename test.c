#include "common.h"
#include "lsfun.h"

int main()
{
	char *msg, *m;
	m = (char *) malloc(200 * sizeof(char));
	msg = m;
	//msg[4] = 0;
	strncpy(m, "hello", 6);
	int val = strcmp(msg, "hello");
	if( val == 0)
		printf("yes\n");
	printf("length:%ld\t%s\n", strlen(m), m);
	printf("length:%ld\t%s\n",strlen(msg), msg);
	printf("length ls :%ld\n", strlen("ls"));
	return 0;
}
