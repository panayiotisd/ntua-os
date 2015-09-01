#include <unistd.h>
#include <string.h>

void zing()
{
	char *str=getlogin();
	int size=strlen(str);
	write(STDOUT_FILENO,"Don't burn yourself out ",24);
	write(STDOUT_FILENO,str,size);
	write(STDOUT_FILENO,"! :P\n",5);
	return;
}
