#include "common_threads.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Initialize a mutex lock for synchronization
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int current = 1;
int n = 5;

/**
 * Child thread prints their number i in a continuous loop.
 * @param arg: thread number
 * @return NULL
 */
void *child(void *arg) {
  int i = *((int *)arg);
  while (1) {
    // Lock the mutex before printing the message
    Pthread_mutex_lock(&lock);

    // Print the message if the current thread number matches i
    if (current == i) {
      printf("%d\n", i);
      sleep(1);
      current = (current % n) + 1;
    }

    // Unlock the mutex after printing the message
    Pthread_mutex_unlock(&lock);
  }
  return NULL;
}

/**
 * Program has N threads and each thread prints
 * their number i in a continuous loop.
 */
int main(int argc, char *argv[]) {
  pthread_t main_thread[n];
  int thread_id[n];

  // Create N threads
  for (int i = 0; i < n; i++) {
    thread_id[i] = i + 1;
    Pthread_create(&main_thread[i], NULL, child, &thread_id[i]);
  }

  // Join waits for the threads to finish
  for (int i = 0; i < n; i++) {
    Pthread_join(main_thread[i], NULL);
  }

  return 0;
}