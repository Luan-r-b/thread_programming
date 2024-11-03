#include <stdlib.h>
#include "worker.h"
#include "bank.h"
#include "server.h"

extern Bank bank;
extern Server server;


void* worker_thread_func(void* arg) {
    WorkerThread* worker = (WorkerThread*)arg;
    
    while (1) {
        pthread_mutex_lock(&worker->lock);
        while (worker->busy == 0) {
            pthread_cond_wait(&worker->cond, &worker->lock);
        }

        printf("Worker %ld started processing request.\n", worker - server.workers);
        // Processar a requisição
        Request* request = worker->request;
        if (request->type == DEPOSIT) {
            deposit(&bank.accounts[request->account_id], request->amount);
        } else if (request->type == TRANSFER) {
            transfer(&bank.accounts[request->from_account_id], &bank.accounts[request->to_account_id], request->amount);
        }
        free(request);
        worker->busy = 0;
        worker->request = NULL;
        // Notificar o servidor que está livre
        pthread_mutex_lock(&server.lock);
        server.free_workers++;
        server.total_operations++;
        pthread_cond_signal(&server.cond);
        pthread_mutex_unlock(&server.lock);
        pthread_mutex_unlock(&worker->lock);
    }
    return NULL;
}
