#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BACKGROUND_PROCESSES 64

int numBackgroundProcesses = 0;
pid_t backgroundProcesses[MAX_BACKGROUND_PROCESSES];

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
	tokens[tokenNo] = NULL;
	return tokens;
}

void sigintHandler(int sig) {
	printf("\n");
	exit(0);
}

void reapBackgroundProcesses() {
	int status;
	pid_t pid;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		printf("Background process %d finished\n", pid);
		for (int i = 0; i < numBackgroundProcesses; i++) {
			if (backgroundProcesses[i] == pid) {
				backgroundProcesses[i] = 0;
				for (int j = i; j < numBackgroundProcesses - 1; j++) {
					backgroundProcesses[j] = backgroundProcesses[j + 1];
				}
				numBackgroundProcesses--;
				break;
			}
		}
	}
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
		reapBackgroundProcesses();

		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		if (line[0] == '\0' || isspace(line[0])) continue;
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);

		if (tokens[0] == NULL) {
			printf("Shell: Incorrect command\n");
			continue;
		}	else if (strcmp(tokens[0], "exit") == 0) {
			for (i = 0; i < numBackgroundProcesses; i++) {
				if (backgroundProcesses[i] != 0) kill(backgroundProcesses[i], SIGINT);
			}

			for (i = 0; i < numBackgroundProcesses; i++) {
				if (waitpid(backgroundProcesses[i], NULL, 0) == -1) {
					perror("waitpid");
					exit(1);
				}
			}

			for(i = 0; tokens[i] != NULL; i++) {
				free(tokens[i]);
			}
			free(tokens);

			exit(0);
		} else if (strcmp(tokens[0], "cd") == 0) {
			if (tokens[1] == NULL) {
				printf("Shell: Incorrect command\n");
				continue;
			}
			if (tokens[2] != NULL) {
				printf("cd: Too many arguments\n");
				continue;
			}
			if (chdir(tokens[1]) == -1) printf("cd: %s: No such file or directory\n", tokens[1]);
		} else {		
			int length = 0;
			int isBgRunning = 0;
			for (i = 0; tokens[i] != NULL; i++) length++;

			if (strcmp(tokens[length - 1], "&") == 0) {
				isBgRunning = 1;
				tokens[length - 1] = NULL;
			}

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
				if (isBgRunning) {
					printf("Shell: Background process %d started\n", rc);
					backgroundProcesses[numBackgroundProcesses++] = rc;
					if (numBackgroundProcesses > MAX_BACKGROUND_PROCESSES) {
						printf("Maximum number of background processes reached\n");
						exit(1);
					}
				} else {
					if (waitpid(rc, NULL, 0) == -1) {
						perror("waitpid");
						exit(1);
					}
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
