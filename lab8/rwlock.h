#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct read_write_lock {
  pthread_mutex_t mutex; // Mutex for the lock
  pthread_cond_t reader; // Condition variable for readers
  pthread_cond_t writer; // Condition variable for writers
  int activeReaders; // Number of active readers
  int activeWriters; // Number of active writers
  int waitingReaders; // Number of waiting readers
  int waitingWriters; // Number of waiting writers
};

void initalizeReadWriteLock(struct read_write_lock * rw);
void readerLock(struct read_write_lock * rw);
void readerUnlock(struct read_write_lock * rw);
void writerLock(struct read_write_lock * rw);
void writerUnlock(struct read_write_lock * rw);
