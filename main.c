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

volatile int shutdown_clients = 0;
volatile int shutdown_workers = 0;
volatile int shutdown_server = 0;

pthread_mutex_t shutdown_mutex = PTHREAD_MUTEX_INITIALIZER;

void perform_graceful_shutdown(pthread_t* client_threads, int num_clients, pthread_t server_thread) {
    printf("1. Signaling clients to stop...\n");
    shutdown_clients=1;
    for (int i = 0; i < num_clients; i++) {
        int join_result = pthread_join(client_threads[i], NULL);
        if (join_result == 0) {
            printf("   - Client %d stopped successfully\n", i);
        } else {
            printf("   - Warning: Failed to join client %d (error: %d)\n", i, join_result);
        }
    }

    pthread_mutex_lock(&request_queue.lock);
    int current_size = request_queue.size;
    pthread_mutex_unlock(&request_queue.lock);
    if (current_size == 0){
        printf("Queue is empty\n");
    }
    else{
        printf("Waiting to empty queue\n");
        while (1)
        {
            pthread_mutex_lock(&request_queue.lock);
            int current_size = request_queue.size;
            pthread_mutex_unlock(&request_queue.lock);
            
            if (current_size == 0) {
                printf("   Queue is empty.\n");
                break;
            }
        }
    }
        
    // 3. Stop server
    printf("3. Stopping server...\n");
    shutdown_server = 1;
    pthread_cond_signal(&server.cond);
    int server_join_result = pthread_join(server_thread, NULL);
    if (server_join_result == 0) {
        printf("   Server stopped successfully\n");
    } else {
        printf("   Warning: Failed to join server thread (error: %d)\n", server_join_result);
    }
    
    printf("4. Initiating shutdown of all workers...\n");
    shutdown_workers = 1;
    for (int i = 0; i < server.num_workers; i++) {
        pthread_mutex_lock(&server.workers[i].lock);
        server.workers[i].busy = 0;  // Mark as not busy to allow exit
        pthread_cond_signal(&server.workers[i].cond);  // Signal to exit wait
        pthread_mutex_unlock(&server.workers[i].lock);
    }

    // Join each worker to ensure they have completed
    for (int i = 0; i < server.num_workers; i++) {
        int join_result = pthread_join(server.workers[i].thread, NULL);
        if (join_result == 0) {
            printf("   - Worker %d stopped successfully\n", i);
        } else {
            printf("   - Warning: Failed to join worker %d (error: %d)\n", i, join_result);
        }
    }
    printf("All workers stopped.\n");

}



int main(int argc, char* argv[]) {
    if (argc != 7) {
        printf("Uso: %s <num_workers> <num_accounts> <num_clients> <sleep_time_client> <sleep_time_operation> <time_execution>\n", argv[0]);
        return -1;
    }

    int num_workers = atoi(argv[1]);
    int num_accounts = atoi(argv[2]);
    int num_clients = atoi(argv[3]);
    //To use usleep *1000 to be in ms the tests
    int sleep_time_client = atoi(argv[4])*1000;
    int sleep_time_operation = atoi(argv[5])*1000; 
    int time_execution = atoi(argv[6]);

    srand(time(NULL));
    bank.time_sleep = sleep_time_operation;

    init_bank(&bank, num_accounts,sleep_time_operation); 
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

    // Perform graceful shutdown
    perform_graceful_shutdown(client_threads, num_clients, server_thread);

    // Last print account_balance
    print_balance(&bank, bank.time_sleep);

    pthread_mutex_lock(&request_queue.lock);
    printf("Final queue size: %d\n", request_queue.size);  // Access under lock
    pthread_mutex_unlock(&request_queue.lock);

    // Encerrar o programa
    printf("Tempo de execução concluído. Encerrando...\n");
    exit(0);
}
