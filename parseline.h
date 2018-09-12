#ifndef PARSELINE_H
#define PARSELINE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

struct stage {
	char input[512];
	char output[512];
	int s_argc;
	char s_argv[10][50];
	char line[512];
};

int parse_line(char *line, struct stage *stages);
int count_tokens(char *line, char *delim);
int end_stage(int start, int linec, char *linev[]);
void print_stages(struct stage *stages, int s_n);

#endif
