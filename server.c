#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "server.h"
#include "bank.h"
#include "request_queue.h"

extern Bank bank;
extern RequestQueue request_queue;

extern int shutdown_server;
extern int shutdown_workers;

void init_server(Server* server, int num_workers) {
    server->workers = (WorkerThread*)malloc(sizeof(WorkerThread) * num_workers);
    server->num_workers = num_workers;
    server->free_workers = num_workers;
    server->total_operations = 0;

    pthread_mutex_init(&server->lock, NULL);
    pthread_cond_init(&server->cond, NULL);

    for (int i = 0; i < num_workers; ++i) {
        pthread_mutex_init(&server->workers[i].lock, NULL);
        pthread_cond_init(&server->workers[i].cond, NULL);
        server->workers[i].busy = 0;
        server->workers[i].request = NULL;
    }
}

void* server_thread_func(void* arg) {
    Server* server = (Server*)arg;
    while(!shutdown_server){
        pthread_mutex_lock(&server->lock);

    

        // Wait until there is a request and a free worker
        while (request_queue.size == 0 || server->free_workers == 0) {
            pthread_cond_wait(&server->cond, &server->lock);
            if(shutdown_server){
                pthread_mutex_unlock(&server->lock);
                return NULL;
            }
        }



        // Get a request from the queue
        Request* request = dequeue_request(&request_queue);

        // Find an available worker
        WorkerThread* worker = NULL;
        for (int i = 0; i < server->num_workers; ++i) {
            if (server->workers[i].busy == 0) {
                worker = &server->workers[i];
                pthread_mutex_lock(&worker->lock);
                worker->busy = 1;
                worker->request = request;
                server->free_workers--;
                pthread_cond_signal(&worker->cond); // Signal to the worker
                pthread_mutex_unlock(&worker->lock);
                printf("Server assigned request to Worker %d\n", i);
                break;
            }
        }

        server->total_operations++;
        if (server->total_operations % 10 == 0) {
            printf("\nServer processed %d total operations.\n", server->total_operations);
            printf("Current bank balances:\n");
            print_balance(&bank, bank.time_sleep);
            printf("\n");
        }
        pthread_mutex_unlock(&server->lock);
    }
    return NULL;
}

