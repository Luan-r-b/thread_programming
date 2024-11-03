#include <stdlib.h>
#include "worker.h"
#include "bank.h"
#include "server.h"

extern Bank bank;
extern Server server;

extern int shutdown_workers;


void* worker_thread_func(void* arg) {
    WorkerThread* worker = (WorkerThread*)arg;

    while (1) {
        pthread_mutex_lock(&worker->lock);
        while (worker->busy == 0 && !shutdown_workers) {
            pthread_cond_wait(&worker->cond, &worker->lock);
        }

        // Exit if shutdown_workers is set
        if (shutdown_workers) {
            pthread_mutex_unlock(&worker->lock);
            break;
        }

        // Process the request
        printf("Worker %ld started processing request.\n", worker - server.workers);
        Request* request = worker->request;
        if (request->type == DEPOSIT) {
            deposit(&bank.accounts[request->account_id], request->amount, bank.time_sleep);
        } else if (request->type == TRANSFER) {
            transfer(&bank.accounts[request->from_account_id], &bank.accounts[request->to_account_id], request->amount, bank.time_sleep);
        }
        free(request);
        worker->busy = 0;
        worker->request = NULL;

        // Notify the server of a free worker
        pthread_mutex_lock(&server.lock);
        server.free_workers++;
        pthread_cond_signal(&server.cond);  // Notify server of the free worker
        pthread_mutex_unlock(&server.lock);
        pthread_mutex_unlock(&worker->lock);
    }
    return NULL;
}

