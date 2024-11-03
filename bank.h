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
    pthread_mutex_t lock;
} Bank;

void init_bank(Bank* bank, int num_accounts);
void deposit(Account* account, double amount);
void transfer(Account* from, Account* to, double amount);
void print_balance(Bank* bank);

#endif // BANK_H
