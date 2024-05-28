#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[]) {
  int rc = fork();
  if (rc < 0) {
    fprintf(stderr, "fork failed\n");
    exit(1);
  } else if (rc == 0) {
    sleep(5);
    printf("I am child, pid: %d\n", getpid());
  } else {
    kill(rc, SIGKILL);
    printf("I am parent of %d, pid: %d\n", rc, getpid());
  }
  return 0;
}