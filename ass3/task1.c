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
    struct shared *shm;
    int pipefd[2];
    pid_t pid;
    char input[100];
    char pipe_var[100];

    shmid = shmget(key_of_shm, size_of_shm, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    shm = (struct shared *)shmat(shmid, NULL, 0);
    if (shm == (struct shared *)-1) {
        perror("shmat");
        exit(1);
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    printf("Provide Your Input From Given Options:\n");
    printf("1. Type a to Add Money\n");
    printf("2. Type w to Withdraw Money\n");
    printf("3. Type c to Check Balance\n");
    scanf("%s", input);

    strncpy(shm->sel, input, sizeof(shm->sel)-1);
    shm->sel[sizeof(shm->sel)-1] = '\0';
    shm->b = 1000;

    printf("\nYour selection: %s\n\n", shm->sel);

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { 
        int bank_balance;

        close(pipefd[0]);

        if (strcmp(shm->sel, "a") == 0) {
            printf("Enter amount to be added:\n");
            scanf("%d", &bank_balance);
            if (bank_balance > 0) {
                shm->b += bank_balance;
                printf("Balance added successfully\n");
                printf("Updated balance after addition:\n%d\n", shm->b);
            } else {
                printf("Adding failed, Invalid amount\n");
            }
        }
        else if (strcmp(shm->sel, "w") == 0) {
            printf("Enter amount to be withdrawn:\n");
            scanf("%d", &bank_balance);
            if (bank_balance > 0 && bank_balance <= shm->b) {
                shm->b -= bank_balance;
                printf("Balance withdrawn successfully\n");
                printf("Updated balance after withdrawal:\n%d\n", shm->b);
            } else {
                printf("Withdrawal failed, Invalid amount\n");
            }
        }
        else if (strcmp(shm->sel, "c") == 0) {
            printf("Your current balance is:\n%d\n", shm->b);
        }
        else {
            printf("Invalid selection\n");
        }

        // Write to pipe
        write(pipefd[1], "Thank you for using", 19);
        close(pipefd[1]);

        // Detach shared memory
        shmdt(shm);
        exit(0);
    }
    else { 
        close(pipefd[1]);

        wait(NULL);

        read(pipefd[0], pipe_var, sizeof(pipe_var));
        printf("%s\n", pipe_var);
        close(pipefd[0]);

        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
    }
    return 0;
}