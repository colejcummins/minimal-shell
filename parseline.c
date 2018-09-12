#define _GNU_SOURCE 

#include "parseline.h"

int parse_line(char *line, struct stage *stages) {
	int s_n, s_c, a_c;
	char *line_tok, *rest, *arg_tok, *arg_rest;
	memset(stages, 0, 512);
	s_n = 0;
	if((s_c = count_tokens(line, "|")) == -1)
		return -1;
	line_tok = strtok_r(line, "|", &rest);
	while(line_tok != NULL) {
		if(s_n >= 10) {
			fprintf(stderr, "pipeline too deep\n");
			return -1;
		}
		if((a_c = count_tokens(line_tok, " ")) == 0) {
			fprintf(stderr, "invalid null command\n");
			return -1;
		}
		if(a_c == -1)
			return -1;
		if(s_n == 0) {
			strcpy(stages[s_n].input, "original stdin");
			strcpy(stages[s_n].output, "original stdout");
		}
		else {
			sprintf(stages[s_n - 1].output, "pipe to stage %d", s_n);
			sprintf(stages[s_n].input, "pipe from stage %d", (s_n - 1));
			strcpy(stages[s_n].output, "original stdout");
		}
		strcpy(stages[s_n].line, line_tok);
		stages[s_n].s_argc = 0;
		arg_tok = strtok_r(line_tok, " ", &arg_rest);
		while(arg_tok != NULL) {
			if(stages[s_n].s_argc == 10) {
				fprintf(stderr, "too many arguments\n");
				return -1;
			}
			if(strcmp(arg_tok, "<") == 0) {
				if(s_n != 0) {
					fprintf(stderr, "ambiguous input\n");
					return -1;
				}
				if(((arg_tok = strtok_r(NULL, " ", &arg_rest)) == NULL) 
					|| (strcmp(stages[s_n].input, "original stdin") != 0)
					|| (strcmp(arg_tok, ">") == 0)
					|| (strcmp(arg_tok, "<") == 0)) {
					fprintf(stderr, "bad input redirection\n");
					return -1;
				}
				strcpy(stages[s_n].input, arg_tok);
			} 
			else if(strcmp(arg_tok, ">") == 0) {
				if(s_n != (s_c - 1)) {
					fprintf(stderr, "ambiguous output\n");
					return -1;
				}
				if(((arg_tok = strtok_r(NULL, " ", &arg_rest)) == NULL) 
					|| (strcmp(stages[s_n].output, "original stdout") != 0)
					|| (strcmp(arg_tok, ">") == 0)
					|| (strcmp(arg_tok, "<") == 0)) {
					fprintf(stderr, "bad output redirection\n");
					return -1;
				}
				strcpy(stages[s_n].output, arg_tok);
			} else {
				strcpy(stages[s_n].s_argv[stages[s_n].s_argc], arg_tok);
				stages[s_n].s_argc++;
			}
			arg_tok = strtok_r(NULL, " ", &arg_rest);
		}
		s_n++;
		line_tok = strtok_r(NULL, "|", &rest);
	}
	return s_c;
}

int count_tokens(char *line, char *delim) {
	char *tok, *rest, *save;
	int count;
	if(line[0] == '\0' || (line[0] == 32 && line[1] == 0)) {
		fprintf(stderr, "invalid null command\n");
		return -1;
	}
	save = malloc(sizeof(line));
	strcpy(save, line);
	tok = strtok_r(save, delim, &rest);
	count = 0;
	if(strcmp(tok, "\n") == 0) {
		fprintf(stderr, "invalid null command\n");
		return -1;
	}
	while(tok != NULL) {
		count++;
		tok = strtok_r(NULL, delim, &rest);
	}	
	return count;
}
	
int end_stage(int start, int linec, char *linev[]) {
	int i, i_pipe;
	i_pipe = linec;
	for(i = start; i < linec; i++) {
		if(strcmp(linev[i], "|") == 0)
			i_pipe = i; 	
	}
	return i_pipe;
}

void print_stages(struct stage *stages, int s_n) {
	int i, j;
	for(i = 0; i < s_n; i++) {
		printf("\n--------\nStage %d: \"%s\"\n--------\n", i , stages[i].line);
		printf("%10s: %s\n", "input", stages[i].input);
		printf("%10s: %s\n", "output", stages[i].output);
		printf("%10s: %d\n", "argc", stages[i].s_argc);
		printf("%10s: ", "argv");
		for(j = 0; j < stages[i].s_argc; j++) {
			if(j > 0)
				printf(",");
			printf("\"%s\"", stages[i].s_argv[j]);
		}
		printf("\n");
	}
}
