#include "rwlock.h"

void initalizeReadWriteLock(struct read_write_lock * rw) {
    // Write the code for initializing your read-write lock.
    rw->activeReaders = 0;
    rw->activeWriters = 0;
    rw->waitingReaders = 0;
    rw->waitingWriters = 0;
    pthread_mutex_init(&rw->mutex, NULL);
    pthread_cond_init(&rw->reader, NULL);
    pthread_cond_init(&rw->writer, NULL);
}

void readerLock(struct read_write_lock * rw) {
    // Write the code for aquiring read-write lock by the reader.
    pthread_mutex_lock(&rw->mutex);
    while(rw->activeWriters > 0) {
        pthread_cond_wait(&rw->reader, &rw->mutex);
    }
    rw->activeReaders++;
    pthread_mutex_unlock(&rw->mutex);
}

void readerUnlock(struct read_write_lock * rw) {
    // Write the code for releasing read-write lock by the reader.
    pthread_mutex_lock(&rw->mutex);
    rw->activeReaders--;
    if(rw->activeReaders == 0 && rw->waitingWriters > 0) {
        pthread_cond_signal(&rw->writer);
    }
    pthread_mutex_unlock(&rw->mutex);
}

void writerLock(struct read_write_lock * rw) {
    // Write the code for aquiring read-write lock by the writer.
    pthread_mutex_lock(&rw->mutex);
    rw->waitingWriters++;
    while(rw->activeReaders > 0 || rw->activeWriters > 0) {
        pthread_cond_wait(&rw->writer, &rw->mutex);
    }
    rw->waitingWriters--;
    rw->activeWriters++;
    pthread_mutex_unlock(&rw->mutex);
}

void writerUnlock(struct read_write_lock * rw) {
    // Write the code for releasing read-write lock by the writer.
    pthread_mutex_lock(&rw->mutex);
    rw->activeWriters--;
    if(rw->waitingWriters > 0) {
        pthread_cond_signal(&rw->writer);
    } else {
        pthread_cond_broadcast(&rw->reader);
    }
    pthread_mutex_unlock(&rw->mutex);
}