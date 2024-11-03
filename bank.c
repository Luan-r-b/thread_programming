#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // For usleep
#include "bank.h"

void init_bank(Bank* bank, int num_accounts) {
    printf("Initializing bank with %d accounts.\n", num_accounts);
    bank->num_accounts = num_accounts;
    bank->accounts = (Account*)malloc(num_accounts * sizeof(Account));

    for (int i = 0; i < num_accounts; i++) {
        bank->accounts[i].id = i;
        bank->accounts[i].balance = 1000.0;
        pthread_mutex_init(&bank->accounts[i].lock, NULL);
        printf("Initialized Account %d with balance %.2f\n", i, bank->accounts[i].balance);
    }
    printf("Bank initialization complete.\n");
}

void deposit(Account* account, double amount) {
    printf("Depositing %.2f into Account %d\n", amount, account->id);
    pthread_mutex_lock(&account->lock);
    account->balance += amount;
    printf("New balance of Account %d after deposit: %.2f\n", account->id, account->balance);
    pthread_mutex_unlock(&account->lock);
    usleep(100000); // Simulate operation time
}

void transfer(Account* from, Account* to, double amount) {
    printf("Transferring %.2f from Account %d to Account %d\n", amount, from->id, to->id);

    Account *first, *second;
    if (from->id < to->id) {
        first = from;
        second = to;
    } else {
        first = to;
        second = from;
    }
    
    //Verify if need to do a conditional signal
    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);

    from->balance -= amount;
    to->balance += amount;

    printf("New balance of Account %d after transfer: %.2f\n", from->id, from->balance);
    printf("New balance of Account %d after transfer: %.2f\n", to->id, to->balance);

    pthread_mutex_unlock(&first->lock);
    pthread_mutex_unlock(&second->lock);
    usleep(100000); // Simulate operation time
}

void print_balance(Bank* bank) {
    printf("Printing balances of all accounts:\n");
    for (int i = 0; i < bank->num_accounts; i++) {
        Account* account = &bank->accounts[i];
        pthread_mutex_lock(&account->lock);
        printf("Account %d: Balance = %.2f\n", account->id, account->balance);
        pthread_mutex_unlock(&account->lock);
    }
    usleep(100000); // Simulate operation time
    printf("Finished printing all balances.\n");
}
