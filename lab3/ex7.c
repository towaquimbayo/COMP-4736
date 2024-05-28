#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sigintHandler(int sig) {
  printf("I will run forever\n");
}

int main(int argc, char *argv[]) {
  if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    printf("Error setting up signal handler\n");
    exit(1);
  }
  while (1) {
    printf("I am running...\n");
    pause();
  }
  return 0;
}