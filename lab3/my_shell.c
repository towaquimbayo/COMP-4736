#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/**
 * Splits the string by space and returns the array of tokens.
 */
char **tokenize(char *line) {
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++) {
		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
			token[tokenIndex] = '\0';
			if (tokenIndex != 0) {
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		}
		else {
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}

void sigintHandler(int sig) {
	printf("\n");
	exit(0);
}

int main(int argc, char* argv[]) {
	char line[MAX_INPUT_SIZE];            
	char **tokens;              
	int i;

	if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    printf("Error setting up signal handler\n");
    exit(1);
  }

	while(1) {
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		if (line[0] == '\0' || isspace(line[0])) continue;
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		if (strcmp(tokens[0], "cd") == 0) {
			if (tokens[0] == NULL || tokens[1] == NULL) {
				printf("Shell: Incorrect command\n");
				continue;
			}
			if (tokens[2] != NULL) {
				printf("cd: Too many arguments\n");
				continue;
			}
			if (chdir(tokens[1]) == -1) printf("cd: %s: No such file or directory\n", tokens[1]);
		} else {
			int rc = fork();
			if (rc < 0) {
				fprintf(stderr, "fork failed\n");
				exit(1);
			} else if (rc == 0) {
				if (execvp(tokens[0], tokens)) {
					printf("Command '%s' not found\n", *tokens);
					exit(1);
				}
			} else {
				if (wait(NULL) == -1) {
					perror("wait");
					exit(1);
				} 
			}
		}
   
		// Freeing the allocated memory	
		for(i = 0; tokens[i] != NULL; i++) {
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}