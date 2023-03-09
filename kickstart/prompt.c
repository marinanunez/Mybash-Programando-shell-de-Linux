#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>

#include "prompt.h"

void show_prompt(void){
	char * cwdbuf = NULL;
	size_t cwdsize = 1024;
	char * cwd = getcwd(cwdbuf, cwdsize);
	char * user = getpwuid(getuid())->pw_name;
	char hostname[_SC_HOST_NAME_MAX+1];
	gethostname(hostname, sizeof(hostname));
	printf("[%s@%s %s]> ", user, hostname, cwd);
	free(cwdbuf);
	free(cwd);
	fflush (stdout);
}
