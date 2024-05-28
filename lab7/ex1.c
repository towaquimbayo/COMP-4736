#include "common_threads.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

// Initialize a counter
static volatile int counter = 0;
// Initialize a mutex lock
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg) {
  int i;
  for (i = 0; i < 1000; i++) {
    // Lock the mutex before updating the counter
    Pthread_mutex_lock(&lock);
    counter = counter + 1;

    // Unlock the mutex after updating the counter
    Pthread_mutex_unlock(&lock);
  }
  return NULL;
}

/**
 * Spawns 10 threads and increments the counter 1000 times in a loop.
 * Print the final value of the counter after all threads have finished.
 * Expected output: 10000
 */
int main(int argc, char *argv[]) {
  pthread_t p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;

  // Create 10 threads
  printf("main: begin (counter = %d)\n", counter);
  Pthread_create(&p1, NULL, worker, NULL);
  Pthread_create(&p2, NULL, worker, NULL);
  Pthread_create(&p3, NULL, worker, NULL);
  Pthread_create(&p4, NULL, worker, NULL);
  Pthread_create(&p5, NULL, worker, NULL);
  Pthread_create(&p6, NULL, worker, NULL);
  Pthread_create(&p7, NULL, worker, NULL);
  Pthread_create(&p8, NULL, worker, NULL);
  Pthread_create(&p9, NULL, worker, NULL);
  Pthread_create(&p10, NULL, worker, NULL);

  // join waits for the threads to finish
  printf("main: waiting for thread to terminate...\n");
  Pthread_join(p1, NULL);
  Pthread_join(p2, NULL);
  Pthread_join(p3, NULL);
  Pthread_join(p4, NULL);
  Pthread_join(p5, NULL);
  Pthread_join(p6, NULL);
  Pthread_join(p7, NULL);
  Pthread_join(p8, NULL);
  Pthread_join(p9, NULL);
  Pthread_join(p10, NULL);
  printf("main: done with both (counter = %d)\n", counter);

  // Check if the final value of the counter is 10000
  assert(counter == 10000);

  return 0;
}