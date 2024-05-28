#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  printf("I am about to run ls\n");
  execvp("ls", argv);
  printf("This line will not be printed\n");
  return 0;
}