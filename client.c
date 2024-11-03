#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "bank.h"
#include "request_queue.h"

// Global request queue
extern RequestQueue request_queue;
extern Bank bank;


void* client_thread_func(void* arg) {

    int sleep_time_client = *(int*)arg;
    int num_accounts = bank.num_accounts;

    while (1) {
        Request* request = (Request*)malloc(sizeof(Request));
        if (request == NULL) {
            perror("Failed to allocate memory for request");
            continue;
        }

        // Randomly choose between DEPOSIT and TRANSFER
        request->type = (rand() % 2 == 0) ? DEPOSIT : TRANSFER;

        if (request->type == DEPOSIT) {
            // Deposit to a random account
            request->account_id = rand() % num_accounts;
            request->amount = ((double)(rand() % 100));
            printf("Client generated DEPOSIT of %.2f to account %d\n", request->amount, request->account_id);
        } else if (request->type == TRANSFER) {
            // Transfer between two random accounts
            request->from_account_id = rand() % num_accounts;
            do {
                request->to_account_id = rand() % num_accounts;
            } while (request->to_account_id == request->from_account_id); // Ensure different accounts for transfer
            request->amount = ((double)(rand() % 100));
            printf("Client generated TRANSFER of %.2f from account %d to account %d\n",
                   request->amount, request->from_account_id, request->to_account_id);
        }

        enqueue_request(&request_queue, request);
        usleep(sleep_time_client);
    }

    return NULL;
}
