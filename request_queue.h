#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include <pthread.h>

typedef enum {
    DEPOSIT,
    TRANSFER
} RequestType;

typedef struct Request {
    RequestType type;
    int account_id;
    int from_account_id;
    int to_account_id;
    double amount;
    struct Request* next;
} Request;

typedef struct {
    Request* head;
    Request* tail;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int size;
} RequestQueue;

void init_request_queue(RequestQueue* queue);
void enqueue_request(RequestQueue* queue, Request* request);
Request* dequeue_request(RequestQueue* queue);

#endif // REQUEST_QUEUE_H
