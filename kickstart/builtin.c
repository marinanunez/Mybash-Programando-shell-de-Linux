#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <assert.h>

#include "builtin.h"
#include "tests/syscall_mock.h"

bool builtin_is_exit(pipeline pipe){
	assert(pipe != NULL);
	scommand scmd = pipeline_front(pipe);
	char * cmd = scommand_front(scmd);
	return (strcmp(cmd, "exit") == 0);
}

bool builtin_is_cd(pipeline pipe){
	assert(pipe != NULL);
	scommand scmd = pipeline_front(pipe);
	char * cmd = scommand_front(scmd);
	return (strcmp(cmd, "cd") == 0);
}

bool builtin_is_internal(pipeline pipe){
	assert(pipe != NULL);
	return builtin_is_exit(pipe) || builtin_is_cd(pipe);
}

void builtin_exec(pipeline pipe){
	assert(builtin_is_internal(pipe));
	if (builtin_is_cd(pipe)) {
		scommand scmd = pipeline_front(pipe);
		scommand_pop_front(scmd);
		char * path = NULL;
		if (scommand_is_empty(scmd)) {
			// ir al home si no hay argumentos
			path = getenv("HOME");
		} else {
			path = scommand_front(scmd);
		}
		int r = chdir(path);
		if (r == -1) {
			fprintf(stderr, "%s: directorio no válido\n", path);
		}
	}
}
