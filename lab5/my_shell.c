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
#define MAX_FOREGROUND_PROCESSES 64
#define MAX_BACKGROUND_PROCESSES 64

volatile sig_atomic_t sigintReceived = 0;
int numForegroundProcesses = 0;
int numBackgroundProcesses = 0;
char **foregroundProcesses[MAX_FOREGROUND_PROCESSES];
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

/**
 * Frees the allocated memory for tokens.
 */
void freeTokens(char **tokens) {
	for (int i = 0; tokens[i] != NULL; i++) {
		free(tokens[i]);
	}
	free(tokens);
}

/**
 * Signal handler for SIGINT (Ctrl+C).
 * Kills all the foreground processes and exits the shell.
 */
void sigintHandler(int sig) {
	printf("\n");
	for (int i = 0; i < numForegroundProcesses; i++) {
		if (foregroundProcesses[i] != NULL) {
			killpg(getpgid(0), SIGINT);
			freeTokens(foregroundProcesses[i]);
			foregroundProcesses[i] = NULL;
		}
	}
	numForegroundProcesses = 0;
	sigintReceived = 1;
	exit(0);
}

/**
 * Reaps the background processes.
 */
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

/**
 * Executes the exit command.
 */
void exitCommand(char **tokens) {
	for (int i = 0; i < numBackgroundProcesses; i++) {
		if (backgroundProcesses[i] != 0) {
			kill(backgroundProcesses[i], SIGINT);
		}
	}

	for (int i = 0; i < numBackgroundProcesses; i++) {
		if (waitpid(backgroundProcesses[i], NULL, 0) == -1) {
			perror("waitpid");
			exit(1);
		}
	}

	freeTokens(tokens);
	exit(0);
}

/**
 * Executes the cd command.
 */
void execCdCommand(char **tokens) {
	if (tokens[1] == NULL) {
		printf("Shell: Incorrect command\n");
		return;
	}

	if (tokens[2] != NULL) {
		printf("cd: Too many arguments\n");
		return;
	}

	if (chdir(tokens[1]) != 0) {
		printf("cd: %s: No such file or directory\n", tokens[1]);
	}
	return;
}

/**
 * Executes the simple Linux command.
 */
void execSimpleCommand(char **tokens) {
	int length = 0;
	int isBgRunning = 0;
	for (int i = 0; tokens[i] != NULL; i++) length++;

	if (strcmp(tokens[length - 1], "&") == 0) {
		isBgRunning = 1;
		tokens[length - 1] = NULL;
	}

	int rc = fork();
	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) {
		setpgid(0, 0);
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

void commandHandling(char **tokens) {
	if (tokens[0] == NULL) {
		printf("Shell: Incorrect command\n");
		return;
	}	else if (strcmp(tokens[0], "exit") == 0) {
		exitCommand(tokens);
	} else if (strcmp(tokens[0], "cd") == 0) {
		execCdCommand(tokens);
	} else {		
		execSimpleCommand(tokens);
	}
}

/**
 * Checks if the foreground execution is serial by
 * checking if the tokens include &&.
 */
int isCmdSerial(char **tokens) {
	for (int i = 0; tokens[i] != NULL; i++) {
		if (strcmp(tokens[i], "&&") == 0) {
			return 1;
		}
	}
	return 0;
}

/**
 * Checks if the foreground execution is parallel by
 * checking if the tokens include &&&.
 */
int isCmdParallel(char **tokens) {
	for (int i = 0; tokens[i] != NULL; i++) {
		if (strcmp(tokens[i], "&&&") == 0) {
			return 1;
		}
	}
	return 0;
}

/**
 * Executes the serial commands.
 */
void serialCommandHandling(char **tokens) {
	if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    printf("Error setting up signal handler\n");
    exit(1);
  }

	int i = 0;
	while (tokens[i] != NULL) {
		if (numForegroundProcesses >= MAX_FOREGROUND_PROCESSES) {
			printf("Maximum number of foreground processes reached\n");
			exit(1);
		}

		char **commandTokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
		int j = 0;
		while (tokens[i] != NULL && strcmp(tokens[i], "&&") != 0) {
			commandTokens[j] = strdup(tokens[i]); // duplicate the string
			i++;
			j++;
		}
		commandTokens[j] = NULL; // terminate the array with NULL

		foregroundProcesses[numForegroundProcesses++] = commandTokens;
		if (tokens[i] != NULL) i++;
	}

	setpgid(0, 0);
	for (i = 0; i < numForegroundProcesses; i++) {
		int rc = fork();
		if (rc < 0) {
			fprintf(stderr, "fork failed\n");
			exit(1);
		} else if (rc == 0) {
			numForegroundProcesses++;
			if (execvp(foregroundProcesses[i][0], foregroundProcesses[i])) {
				printf("Command '%s' not found\n", *foregroundProcesses[i]);
				exit(1);
			}
		} else {
			if (waitpid(rc, NULL, 0) == -1) {
				perror("waitpid");
				exit(1);
			}
			freeTokens(foregroundProcesses[i]);
			foregroundProcesses[i] = NULL;
			numForegroundProcesses--;
		}
	}
	numForegroundProcesses = 0;
}

/**
 * Executes the parallel commands.
 */
void parallelCommandHandling(char **tokens) {
	if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    printf("Error setting up signal handler\n");
    exit(1);
  }

	// Stores the process IDs of the parallel commands
	pid_t pids[numForegroundProcesses];
	int i = 0;
	while (tokens[i] != NULL) {
		if (numForegroundProcesses >= MAX_FOREGROUND_PROCESSES) {
			printf("Maximum number of foreground processes reached\n");
			exit(1);
		}

		char **commandTokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
		int j = 0;
		while (tokens[i] != NULL && strcmp(tokens[i], "&&&") != 0) {
			commandTokens[j] = strdup(tokens[i]); // duplicate the string
			i++;
			j++;
		}
		commandTokens[j] = NULL; // terminate the array with NULL

		foregroundProcesses[numForegroundProcesses++] = commandTokens;
		if (tokens[i] != NULL) i++;
	}

	setpgid(0, 0);
	for (i = 0; i < numForegroundProcesses; i++) {
		int rc = fork();
		if (rc < 0) {
			fprintf(stderr, "fork failed\n");
			exit(1);
		} else if (rc == 0) {
			if (execvp(foregroundProcesses[i][0], foregroundProcesses[i])) {
				printf("Command '%s' not found\n", *foregroundProcesses[i]);
				exit(1);
			}
		} else {
			pids[i] = rc;
		}
	}
	
	for (i = 0; i < numForegroundProcesses; i++) {
		if (waitpid(pids[i], NULL, 0) == -1) {
			perror("waitpid");
			exit(1);
		}
		freeTokens(foregroundProcesses[i]);
		foregroundProcesses[i] = NULL;
		pids[i] = 0;
	}
	numForegroundProcesses = 0;
}

int main(int argc, char* argv[]) {
	char line[MAX_INPUT_SIZE];
	char **tokens;

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

		if (isCmdSerial(tokens)) {
			serialCommandHandling(tokens);
		} else if (isCmdParallel(tokens)) {
			parallelCommandHandling(tokens);
		} else {
			commandHandling(tokens);
		}

		// Freeing the allocated memory	
		freeTokens(tokens);
	}
	return 0;
}
