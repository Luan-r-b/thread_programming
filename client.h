#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>

// The client thread function, which will generate random requests and enqueue them
void* client_thread_func(void* arg);

#endif // CLIENT_H
    