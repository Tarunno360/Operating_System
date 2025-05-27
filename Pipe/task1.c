#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#define size_of_shm 1024
#define key_of_shm 1234

struct shared {
    char sel[100];
    int b;
};

int main() {
    int shmid;
    struct shared *shm_data_store;
    int pipe_arr[2];
    pid_t pid;
    char input_data[100];
    char pipe_var[100];

    shmid = shmget(key_of_shm, size_of_shm, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    shm_data_store = (struct shared *)shmat(shmid, NULL, 0);
    if (shm_data_store == (struct shared *)-1) {
        perror("shmat");
        exit(1);
    }

    if (pipe(pipe_arr) == -1) {
        perror("pipe");
        exit(1);
    }

    printf("Provide Your Input From Given Options:\n");
    printf("1. Type a to Add Money\n");
    printf("2. Type w to Withdraw Money\n");
    printf("3. Type c to Check Balance\n");
    scanf("%s", input_data);

    strncpy(shm_data_store->sel, input_data, sizeof(shm_data_store->sel)-1);
    shm_data_store->sel[sizeof(shm_data_store->sel)-1] = '\0';
    shm_data_store->b = 1000;

    printf("\nYour selection: %s\n\n", shm_data_store->sel);

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { 
        int bank_balance;

        close(pipe_arr[0]);

        if (strcmp(shm_data_store->sel, "a") == 0) {
            printf("Enter amount to be added:\n");
            scanf("%d", &bank_balance);
            if (bank_balance > 0) {
                shm_data_store->b += bank_balance;
                printf("Balance added successfully\n");
                printf("Updated balance after addition:\n%d\n", shm_data_store->b);
            } else {
                printf("Adding failed, Invalid amount\n");
            }
        }
        else if (strcmp(shm_data_store->sel, "w") == 0) {
            printf("Enter amount to be withdrawn:\n");
            scanf("%d", &bank_balance);
            if (bank_balance > 0 && bank_balance <= shm_data_store->b) {
                shm_data_store->b -= bank_balance;
                printf("Balance withdrawn successfully\n");
                printf("Updated balance after withdrawal:\n%d\n", shm_data_store->b);
            } else {
                printf("Withdrawal failed, Invalid amount\n");
            }
        }
        else if (strcmp(shm_data_store->sel, "c") == 0) {
            printf("Your current balance is:\n%d\n", shm_data_store->b);
        }
        else {
            printf("Invalid selection\n");
        }

        write(pipe_arr[1], "Thank you for using", 19);
        close(pipe_arr[1]);

        shmdt(shm_data_store);
        exit(0);
    }
    else { 
        close(pipe_arr[1]);

        wait(NULL);

        read(pipe_arr[0], pipe_var, sizeof(pipe_var));
        printf("%s\n", pipe_var);
        close(pipe_arr[0]);

        shmdt(shm_data_store);
        shmctl(shmid, IPC_RMID, NULL);
    }
    return 0;
}