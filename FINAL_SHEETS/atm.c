#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define WITHDRAW_AMOUNT 200
#define INITIAL_BALANCE 300

int balance = INITIAL_BALANCE;
sem_t atm_semaphore;
void* use_atm(void* arg) {
    int user_id = *((int*)arg);
    printf("User %d is waiting to access the ATM.\n", user_id);
    sem_wait(&atm_semaphore);  
    printf("User %d is using the ATM.\n", user_id);
    sleep(1); 

    if (balance >= WITHDRAW_AMOUNT) {
        balance -= WITHDRAW_AMOUNT;
        printf("User %d withdrew $%d. Remaining balance: $%d\n", user_id, WITHDRAW_AMOUNT, balance);
    } else {
        printf("User %d tried to withdraw money but had insufficient funds. Remaining balance: $%d\n", user_id, balance);
    }
    printf("User %d has finished using the ATM.\n", user_id);

    sem_post(&atm_semaphore); 
    return NULL;
}

int main() {
    pthread_t users[3];
    int user_ids[3] = {1, 2, 3};

    sem_init(&atm_semaphore, 0, 1);
    for (int i = 0; i < 3; i++) {
        pthread_create(&users[i], NULL, use_atm, &user_ids[i]);
        sleep(1);  
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(users[i], NULL);
    }

    sem_destroy(&atm_semaphore);
    return 0;
}
