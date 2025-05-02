#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHM_SIZE 1024
#define SHM_KEY 1234

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
    char pipe_buf[100];

    // Create shared memory
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // Attach shared memory
    shm = (struct shared *)shmat(shmid, NULL, 0);
    if (shm == (struct shared *)-1) {
        perror("shmat");
        exit(1);
    }

    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Prompt user for input
    printf("Provide Your Input From Given Options:\n");
    printf("1. Type a to Add Money\n");
    printf("2. Type w to Withdraw Money\n");
    printf("3. Type c to Check Balance\n");
    scanf("%s", input);

    // Store user input in shared memory
    strncpy(shm->sel, input, sizeof(shm->sel)-1);
    shm->sel[sizeof(shm->sel)-1] = '\0';
    shm->b = 1000;

    // Print user selection
    printf("\nYour selection: %s\n\n", shm->sel);

    // Create child process
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // Child process (opr)
        int amount;

        // Close read end of pipe
        close(pipefd[0]);

        // Process user selection
        if (strcmp(shm->sel, "a") == 0) {
            printf("Enter amount to be added:\n");
            scanf("%d", &amount);
            if (amount > 0) {
                shm->b += amount;
                printf("Balance added successfully\n");
                printf("Updated balance after addition:\n%d\n", shm->b);
            } else {
                printf("Adding failed, Invalid amount\n");
            }
        }
        else if (strcmp(shm->sel, "w") == 0) {
            printf("Enter amount to be withdrawn:\n");
            scanf("%d", &amount);
            if (amount > 0 && amount <= shm->b) {
                shm->b -= amount;
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
    else { // Parent process (home)
        // Close write end of pipe
        close(pipefd[1]);

        // Wait for child to terminate
        wait(NULL);

        // Read from pipe
        read(pipefd[0], pipe_buf, sizeof(pipe_buf));
        printf("%s\n", pipe_buf);
        close(pipefd[0]);

        // Detach and remove shared memory
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}