#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include "worker.h"
#include "request_queue.h"

typedef struct {
    WorkerThread* workers;
    int num_workers;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int free_workers;
    int total_operations;
} Server;

void init_server(Server* server, int num_workers);
void* server_thread_func(void* arg);

#endif // SERVER_H
