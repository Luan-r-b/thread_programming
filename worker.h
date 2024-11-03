#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>
#include "request_queue.h"

typedef struct WorkerThread {
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int busy;
    Request* request;
} WorkerThread;

void* worker_thread_func(void* arg);

#endif // WORKER_H
