#include "common_threads.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Initialize a mutex lock for synchronization
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// Initialize a condition variable for synchronization
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int done = 0;

/**
 * Child thread sleeps for a random interval between 1 and 10 seconds
 * and prints the message "I am thread i" and exit.
 * @param arg: thread number
 * @return NULL
 */
void *child(void *arg) {
  int i = *((int *)arg);
  int sleep_time = rand() % 10 + 1;

  // Lock the mutex before updating the counter
  Pthread_mutex_lock(&lock);

  // Sleep for a random interval between 1 and 10 seconds
  printf("I am thread %d. I will sleep for %d seconds\n", i, sleep_time);
  sleep(sleep_time);
  printf("I am thread %d\n", i);

  // Increment the done counter and signal the condition variable
  done++;
  Pthread_cond_signal(&cond);

  // Unlock the mutex after updating the counter
  Pthread_mutex_unlock(&lock);
  return NULL;
}

/**
 * Main default thread spawns N threads.
 * Each thread sleeps for a random interval between 1 and 10 seconds
 * and prints the message "I am thread i" and exit.
 * Main thread waits for all threads to finish.
 */
int main(int argc, char *argv[]) {
  int n = 10;
  pthread_t main_thread[n];
  int thread_id[n];

  // Create N threads
  for (int i = 0; i < n; i++) {
    thread_id[i] = i;
    Pthread_create(&main_thread[i], NULL, child, &thread_id[i]);
  }

  // Join waits for the threads to finish
  for (int i = 0; i < n; i++) {
    Pthread_mutex_lock(&lock);
    while (done == i) {
      Pthread_cond_wait(&cond, &lock);
    }
    Pthread_mutex_unlock(&lock);
  }

  // Main thread
  printf("I am the main thread\n");
  return 0;
}