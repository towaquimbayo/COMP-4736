#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "fork failed\n");
    exit(1);
  } else if (rc == 0) {
    printf("I am child, pid: %d\n", getpid());
  } else {
    int rc_wait = wait(NULL);
    printf("I am parent of %d, pid: %d\n", rc_wait, getpid());
  }
  return 0;
}