#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "server.h"
#include "bank.h"
#include "request_queue.h"

extern Bank bank;
extern RequestQueue request_queue;


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
    while (1) {

        pthread_mutex_lock(&server->lock);
        while (request_queue.size == 0 || server->free_workers == 0) {
            pthread_cond_wait(&server->cond, &server->lock);
        }
        // Obter uma requisição
        Request* request = dequeue_request(&request_queue);
        // Encontrar um trabalhador livre
        WorkerThread* worker = NULL;
        for (int i = 0; i < server->num_workers; ++i) {
            pthread_mutex_lock(&server->workers[i].lock);
            if (server->workers[i].busy == 0) {
                worker = &server->workers[i];
                worker->busy = 1;
                worker->request = request;
                server->free_workers--;
                pthread_cond_signal(&worker->cond);
                pthread_mutex_unlock(&server->workers[i].lock);
                // Log the assignment
                printf("Server assigned request to Worker %d\n", i);
                break;
            }
            pthread_mutex_unlock(&server->workers[i].lock);
        }
        printf(server->total_operations);
        server->total_operations++;
        // Verificar se é hora de imprimir o balanço geral
        if (server->total_operations % 10 == 0 && server->total_operations != 0) {
            printf("\nBalanço geral após %d operações:\n", server->total_operations);
            print_balance(&bank);
            printf("\n");
        }
        pthread_mutex_unlock(&server->lock);
    }
    return NULL;
}
