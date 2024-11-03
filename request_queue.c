#include <stdlib.h>
#include "request_queue.h"
#include "server.h"

extern Server server;

void init_request_queue(RequestQueue* queue) {
    queue->head = queue->tail = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void enqueue_request(RequestQueue* queue, Request* request) {
    pthread_mutex_lock(&queue->lock);
    request->next = NULL;
    if (queue->tail == NULL) {
        queue->head = queue->tail = request;
    } else {
        queue->tail->next = request;
        queue->tail = request;
    }
    queue->size++;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->lock);

    // Signal the server condition variable
    pthread_mutex_lock(&server.lock);
    pthread_cond_signal(&server.cond);
    pthread_mutex_unlock(&server.lock);

}

Request* dequeue_request(RequestQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    while (queue->size == 0) {
        pthread_cond_wait(&queue->cond, &queue->lock);
    }
    Request* request = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL)
        queue->tail = NULL;
    queue->size--;
    pthread_mutex_unlock(&queue->lock);
    return request;
}
