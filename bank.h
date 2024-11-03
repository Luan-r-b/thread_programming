#ifndef BANK_H
#define BANK_H

#include <pthread.h>

typedef struct {
    int id;
    double balance;
    pthread_mutex_t lock;
} Account;

typedef struct {
    Account* accounts;
    int num_accounts;
    int time_sleep;
    pthread_mutex_t lock;
} Bank;

void init_bank(Bank* bank, int num_accounts,int time_sleep);
void deposit(Account* account, double amount,int time_sleep);
void transfer(Account* from, Account* to, double amount,int time_sleep);
void print_balance(Bank* bank,int time_sleep);

#endif // BANK_H
