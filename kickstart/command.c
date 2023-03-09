#include <stdio.h>
#include <glib.h>
#include <assert.h>

#include "command.h"
#include "strextra.h"


/********** COMANDO SIMPLE **********/

/* Estructura correspondiente a un comando simple.
 * Es una 3-upla del tipo ([char*], char* , char*).
 */

struct scommand_s {
	GSList *args;
	char * redir_in;
	char * redir_out;
};


scommand scommand_new(void){
  scommand result = malloc(sizeof(struct scommand_s));
	result->args = NULL;
	result->redir_in = NULL;
	result->redir_out = NULL;
	assert(result != NULL && scommand_is_empty(result)
		&& scommand_get_redir_in(result) == NULL
		&& scommand_get_redir_out(result) == NULL);
	return result;
}

scommand scommand_destroy(scommand self){
	assert(self != NULL);
	g_slist_free_full(self->args, free);
	free(self->redir_in);
	free(self->redir_out);
	free(self);
	return NULL;
}

void scommand_push_back(scommand self, char * argument){
	assert(self != NULL && argument != NULL);
	self->args = g_slist_append(self->args, argument);
	assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
	assert(self != NULL && !scommand_is_empty(self));
	free(self->args->data);
	self->args = g_slist_delete_link(self->args, self->args);
}

void scommand_set_redir_in(scommand self, char * filename){
	assert(self != NULL);
	free(self->redir_in);
	self->redir_in = filename;
}

void scommand_set_redir_out(scommand self, char * filename){
	assert(self != NULL);
	free(self->redir_out);
	self->redir_out = filename;
}

bool scommand_is_empty(const scommand self){
	assert(self != NULL);
	return (g_slist_length(self->args) == 0);
}

unsigned int scommand_length(const scommand self){
	assert(self != NULL);
	return g_slist_length(self->args);
}

char * scommand_front(const scommand self){
	assert(self != NULL && !scommand_is_empty(self));
	char * front = g_slist_nth_data(self->args, 0);
	assert(front != NULL);
	return front;
}

char * scommand_get_redir_in(const scommand self){
	assert(self != NULL);
	return self->redir_in;
}

char * scommand_get_redir_out(const scommand self){
	assert(self != NULL);
	return self->redir_out;
}

char * scommand_to_string(const scommand self){
	assert(self != NULL);
	char * str = NULL;
	// si el comando simple esta vacio, devolvemos un string vacio
	if (scommand_is_empty(self)){
		str = malloc(sizeof(char));
		*str = '\0';
	} else {
		str = strdup(g_slist_nth_data(self->args, 0));
		for (GSList *i = g_slist_next(self->args); i != NULL; i = g_slist_next(i)) {
			str = strmerge_and_free(str, strdup(" "));
			str = strmerge_and_free(str, strdup(i->data));
		}
		if (scommand_get_redir_in(self) != NULL) {
			str = strmerge_and_free(str, strdup(" < "));
			str = strmerge_and_free(str, strdup(scommand_get_redir_in(self)));
		}
		if (scommand_get_redir_out(self) != NULL) {
			str = strmerge_and_free(str, strdup(" > "));
			str = strmerge_and_free(str, strdup(scommand_get_redir_out(self)));
		}
	}
	assert(scommand_is_empty(self) || scommand_get_redir_in(self) == NULL
		|| scommand_get_redir_out(self) == NULL || strlen(str) > 0);
	return str;
}





/********** COMANDO PIPELINE **********/

/* Estructura correspondiente a un comando pipeline.
 * Es un 2-upla del tipo ([scommand], bool)
 */

struct pipeline_s {
	GSList *scmds;
	bool wait;
};



pipeline pipeline_new(void){
	pipeline result = malloc(sizeof(struct pipeline_s));
	result->scmds = NULL;
	result->wait = true;
	assert(result != NULL && pipeline_is_empty(result)
		&& pipeline_get_wait(result));
	return result;
}

pipeline pipeline_destroy(pipeline self){
	assert(self != NULL);
	for (GSList *i = self->scmds; i != NULL; i = g_slist_next(i)) {
		scommand_destroy(i->data);
	}
	g_slist_free(self->scmds);
	free(self);
	return NULL;
}

void pipeline_push_back(pipeline self, scommand sc){
	assert(self != NULL && sc != NULL);
	self->scmds = g_slist_append(self->scmds, sc);
	assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self){
	assert(self != NULL && !pipeline_is_empty(self));
	scommand_destroy(self->scmds->data);
	gpointer data = g_slist_nth_data(self->scmds, 0);
	self->scmds = g_slist_remove(self->scmds, data);
}

void pipeline_set_wait(pipeline self, const bool w){
	assert(self != NULL);
	self->wait = w;
}

bool pipeline_is_empty(const pipeline self){
	assert(self != NULL);
	return (g_slist_length(self->scmds) == 0);
}

unsigned int pipeline_length(const pipeline self){
	assert(self != NULL);
	return g_slist_length(self->scmds);
}

scommand pipeline_front(const pipeline self){
	assert(self != NULL && !pipeline_is_empty(self));
	scommand scmd = self->scmds->data;
	assert(scmd != NULL);
	return scmd;
}

bool pipeline_get_wait(const pipeline self){
	assert(self != NULL);
	return self->wait;
}

char * pipeline_to_string(const pipeline self){
	assert(self != NULL);
	char * str = NULL;
	// si el pipeline esta vacio, devolvemos un string vacio
	if (pipeline_is_empty(self)){
		str = malloc(sizeof(char));
		*str = '\0';
	} else {
		str = scommand_to_string(self->scmds->data);
		for (GSList *i = self->scmds->next; i != NULL; i = g_slist_next(i)) {
			str = strmerge_and_free(str, strdup(" | "));
			str = strmerge_and_free(str, scommand_to_string(i->data));
		}
		if (!pipeline_get_wait(self)) {
			str = strmerge_and_free(str, strdup(" &"));
		}
	}
	assert(pipeline_is_empty(self) || pipeline_get_wait(self)
		|| strlen(str) > 0);
	return str;
}


