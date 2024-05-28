#include "common_threads.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

// Initialize a mutex lock
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Child thread prints the message "I am thread i" and exit.
 * @param arg: thread number
 * @return NULL
 */
void *child(void *arg) {
  Pthread_mutex_lock(&lock);
  int i = *((int *)arg);
  printf("I am thread %d\n", i);
  Pthread_mutex_unlock(&lock);
  return NULL;
}

/**
 * Main default thread spawns N threads.
 * Each thread prints the message "I am thread i" and exit.
 * Main thread waits for all threads to finish
 * and prints "I am the main thread" and exit.
 */
int main(int argc, char *argv[]) {
  int n = 100;
  pthread_t main_thread[n];
  int thread_id[n];

  // Create N threads
  for (int i = 0; i < n; i++) {
    thread_id[i] = i;
    Pthread_create(&main_thread[i], NULL, child, &thread_id[i]);
  }

  // Join waits for the threads to finish
  for (int i = 0; i < n; i++) {
    Pthread_join(main_thread[i], NULL);
  }

  // Main thread
  printf("I am the main thread\n");
  return 0;
}