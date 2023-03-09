#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "command.h"
#include "execute.h"
#include "builtin.h"
#include "tests/syscall_mock.h"

static char ** scommand_to_argv(scommand self) {
	assert(self != NULL);
	unsigned int sc_len = scommand_length(self);
	char ** argv = malloc(sizeof(char*) * (sc_len + 1));
	unsigned int i = 0;
	while (!scommand_is_empty(self)) {
		argv[i] = strdup(scommand_front(self));
		scommand_pop_front(self);
		i++;
	}
	argv[i] = NULL;
	return argv;
}

static void execute_scommand(scommand self) {
	assert(self != NULL);
	char * cmd = strdup(scommand_front(self));
	unsigned int sc_len = scommand_length(self);
	char ** argv = scommand_to_argv(self);
	if (scommand_get_redir_out(self)) {
		char * redir_out = scommand_get_redir_out(self);
		int fd = open(redir_out, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
		if (fd == -1) {
			fprintf(stderr, "No se pudo escribir al archivo: %s\n", redir_out);
			exit(1);
		} else {
			dup2(fd, 1);
		}
		close(fd);
	}
	if (scommand_get_redir_in(self)) {
		char * redir_in = scommand_get_redir_in(self);
		int fd = open(redir_in, O_RDONLY, S_IRUSR);
		if (fd == -1) {
			fprintf(stderr, "No se pudo leer el archivo: %s\n", redir_in);
			exit(1);
		} else {
			dup2(fd, 0);
		}
		close(fd);
	}
	execvp(cmd, argv);
	fprintf(stderr, "%s: comando no válido\n", cmd);
	free(cmd);
	for (unsigned int i = 0; i < sc_len; i++) {
		free(argv[i]);
	}
	free(argv);
	exit(1);
}

void execute_pipeline(pipeline apipe){
	assert(apipe != NULL);
	if (pipeline_length(apipe) == 1) {
		if (builtin_is_internal(apipe)) {
			builtin_exec(apipe);
		} else {
			pid_t pid = fork();
			if (pid == 0) {
				execute_scommand(pipeline_front(apipe));
			} else {
				if (pipeline_get_wait(apipe)) {
					waitpid(pid, NULL, 0);
				}
			}
		}
	} else if (pipeline_length(apipe) == 2) {
		if (builtin_is_internal(apipe)) {
			fprintf(stderr, "No se puede pipear un comando interno\n");
		} else {
			int p[2];
			pipe(p);
			pid_t pid1 = fork();
			if (pid1 == 0) {
				dup2(p[1], 1);
				close(p[0]);
				close(p[1]);
				execute_scommand(pipeline_front(apipe));
			}
			pipeline_pop_front(apipe);
			pid_t pid2 = fork();
			if (pid2 == 0) {
				dup2(p[0], 0);
				close(p[0]);
				close(p[1]);
				scommand scmd = pipeline_front(apipe);
				execute_scommand(scmd);
			}
			close(p[0]);
			close(p[1]);
			waitpid(pid1, NULL, 0);
			if (pipeline_get_wait(apipe)) {
				waitpid(pid2, NULL, 0);
			}
		}
	}
}
