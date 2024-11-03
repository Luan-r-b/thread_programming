#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "bank.h"
#include "request_queue.h"
#include "worker.h"
#include "server.h"
#include "client.h"

// Variáveis globais
Bank bank;
Server server;
RequestQueue request_queue; 


int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Uso: %s <num_workers> <num_clients> <sleep_time_client> <sleep_time_operation> <time_execution>\n", argv[0]);
        return -1;
    }

    int num_workers = atoi(argv[1]);
    int num_clients = atoi(argv[2]);
    //To use usleep *1000 to be in ms the tests
    int sleep_time_client = atoi(argv[3])*1000;
    int sleep_time_operation = atoi(argv[4])*1000; 
    int time_execution = atoi(argv[5]);

    srand(time(NULL));

    init_bank(&bank, num_clients); 
    init_request_queue(&request_queue);
    init_server(&server, num_workers);

    // Iniciar threads trabalhadoras
    for (int i = 0; i < num_workers; ++i) {
        pthread_create(&server.workers[i].thread, NULL, worker_thread_func, &server.workers[i]);
    }

    // Iniciar thread do servidor
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_thread_func, &server);

    // Iniciar threads clientes
    pthread_t* client_threads = (pthread_t*)malloc(sizeof(pthread_t) * num_clients);
    for (int i = 0; i < num_clients; ++i) {
        pthread_create(&client_threads[i], NULL, client_thread_func, &sleep_time_client);
    }

    // Executar por um tempo determinado
    sleep(time_execution);

    pthread_mutex_lock(&request_queue.lock);
    printf("Final queue size: %d\n", request_queue.size);  // Access under lock
    pthread_mutex_unlock(&request_queue.lock);

    // Encerrar o programa
    printf("Tempo de execução concluído. Encerrando...\n");
    exit(0);
}
