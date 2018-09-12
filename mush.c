#include "parseline.h"

#define READ 0
#define WRITE 1

int read_line(int argc, char *line, FILE *file);
int build_pipeline(struct stage *stages, int s_n, sigset_t o_mask);
void exec_ready(char argv[10][50], int argc);
void sig_handler(int signo);

int main(int argc, char *argv[]) {
	int s_n, i;
	sigset_t n_mask, o_mask;
	FILE *file;
	char line[512];
	struct stage stages[10];
	if(argc > 2) {
		fprintf(stderr, "usage: mush [ file ]\n");
		return 1;
	} if(argc == 2) 
		file = fopen(argv[1], "r");
	else 
		file = stdin;
	sigemptyset(&n_mask);
	sigaddset(&n_mask, SIGINT);
	sigprocmask(SIG_BLOCK, &n_mask, &o_mask);
	if(argc == 1)
		printf("8-P ");
	read_line(argc, line, file);
	while(line[0] != 0) {
		for(i = 0; i < 512; i++) {
			if(line[i] == '\n')
				line[i] = 0;
		}
		if(line[0] != 0)
			s_n = parse_line(line, stages);
		if(s_n != -1 && line[0] != 0) { 
			/* print_stages(stages, s_n); */
			build_pipeline(stages, s_n, o_mask);
		}
		signal(SIGINT, sig_handler);
		memset(line, 0, 512);
		memset(stages, 0, (int)sizeof(stages));
		if(argc == 1)
			printf("8-P ");
		if(read_line(argc, line, file) != 0);
			continue;
	} 
	printf("\n");
	return 0;
}

void sig_handler(int signo) {
	printf("\n");
}

int read_line(int argc, char *line, FILE *file) {
	int c, i;
	if(argc == 2) {
		for(i = 0; (c = fgetc(file)) != '\n' && c != EOF; i++) {
			if(i >= 512) {
				fprintf(stderr, "line too long\n");
				return 1;
			}
			line[i] = c;
		}
	} else {
		fgets(line, 512, file);
	}
	return 0;
}

int build_pipeline(struct stage *stages, int s_n, sigset_t o_mask) {
	int i, j, pipes[9][2], ch_in, ch_out, o_flags, o_mode;
	o_flags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWUSR | S_IROTH | S_IWOTH;
	o_mode = O_WRONLY | O_CREAT | O_TRUNC;
	if(strcmp(stages[0].s_argv[0], "cd") == 0) {
		if(stages[0].s_argc != 2) {
			fprintf(stderr, "usage: cd [ dir ]\n");
			return 1;
		} if(chdir(stages[0].s_argv[1]) == -1) {
			fprintf(stderr, "cd: no such directory\n");
			return 1;
		}
		return 0;
	} 
	
	for(i = 0; i < s_n; i++) {
		pipe(pipes[i]);
		if(fork() == 0) {
			sigprocmask(SIG_SETMASK, &o_mask, NULL);
			if(strcmp(stages[i].input, "original stdin") != 0) {
				if(strncmp(stages[i].input, "pipe", 4) == 0)
					dup2(pipes[i - 1][READ], STDIN_FILENO);
				else {
					if((ch_in = open(stages[i].input, O_RDONLY)) == -1) {
						fprintf(stderr, "%s: open error\n", stages[i].input);
						return 1;
					}
					dup2(ch_in, STDIN_FILENO);
				}
			} if(strcmp(stages[i].output, "original stdout") != 0) {
				if(strncmp(stages[i].output, "pipe", 4) == 0)
					dup2(pipes[i][WRITE], STDOUT_FILENO);
				else {
					if((ch_out = open(stages[i].output, o_mode, o_flags)) == -1) {
						fprintf(stderr, "%s: open error\n", stages[i].output);
						return 1;
					}
					dup2(ch_out, STDOUT_FILENO);
				}
			}
			for(j = 0; j < s_n; j++) {
				if(pipes[j][READ] != 0)
					close(pipes[j][READ]);
				if(pipes[j][WRITE] != 0)
					close(pipes[j][WRITE]);
			}
			exec_ready(stages[i].s_argv, stages[i].s_argc);
		} else {
			wait(NULL);
		}
	}
	return 0;
}

void exec_ready(char argv[10][50], int argc) {
	char *new_argv[10];
	int i;
	for(i = 0; i < argc; i++) {
		new_argv[i] = malloc(50);
		strcpy(new_argv[i], argv[i]);
	}
	execvp(new_argv[0], new_argv);
	fprintf(stderr, "failed to execute\n");
	exit(1);
}
