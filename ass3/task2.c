#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

// Message structure for the message queue
struct msg {
    long int type;
    char txt[6];
};

int main() {
    key_t key;
    int msgid;
    struct msg message;
    pid_t pid_otp, pid_mail;
    char workspace[10];

    // Generate a unique key for the message queue
    key = ftok(".", 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Create message queue
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    // Log in process: Get workspace name
    printf("Please enter the workspace name:\n");
    scanf("%s", workspace);

    if (strcmp(workspace, "cse321") != 0) {
        printf("Invalid workspace name\n");
        // Remove message queue before exiting
        msgctl(msgid, IPC_RMID, NULL);
        exit(0);
    }

    // Log in process: Send workspace name to OTP generator
    message.type = 1; // Type for OTP generator
    strncpy(message.txt, "cse321", 6);
    if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("Workspace name sent to otp generator from log in: %s\n", message.txt);

    // Fork to create OTP generator process
    pid_otp = fork();
    if (pid_otp < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_otp == 0) { // OTP generator process
        // Read workspace name from log in
        if (msgrcv(msgid, &message, sizeof(message.txt), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        printf("\nOTP generator received workspace name from log in: %s\n", message.txt);

        // Generate OTP (process ID) and send to log in
        int otp = getpid();
        snprintf(message.txt, 6, "%d", otp);
        message.type = 2; // Type for log in
        if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        printf("\nOTP sent to log in from OTP generator: %s\n", message.txt);

        // Send same OTP to mail process
        message.type = 3; // Type for mail
        if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        printf("OTP sent to mail from OTP generator: %s\n", message.txt);

        // Fork to create mail process
        pid_mail = fork();
        if (pid_mail < 0) {
            perror("fork");
            exit(1);
        }

        if (pid_mail == 0) { // Mail process
            // Read OTP from OTP generator
            if (msgrcv(msgid, &message, sizeof(message.txt), 3, 0) == -1) {
                perror("msgrcv");
                exit(1);
            }
            printf("\nMail received OTP from OTP generator: %s\n", message.txt);

            // Send same OTP to log in
            message.type = 4; // Type for log in from mail
            if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
            printf("OTP sent to log in from mail: %s\n", message.txt);

            // Mail process terminates
            exit(0);
        }

        // OTP generator waits for mail to terminate
        wait(NULL);
        // OTP generator terminates
        exit(0);
    }

    // Log in process waits for OTP generator to terminate
    wait(NULL);

    // Log in process: Read OTP from OTP generator
    if (msgrcv(msgid, &message, sizeof(message.txt), 2, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    char otp_from_generator[6];
    strncpy(otp_from_generator, message.txt, 6);
    printf("\nLog in received OTP from OTP generator: %s\n", otp_from_generator);

    // Log in process: Read OTP from mail
    if (msgrcv(msgid, &message, sizeof(message.txt), 4, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    printf("Log in received OTP from mail: %s\n", message.txt);

    // Compare OTPs
    if (strcmp(otp_from_generator, message.txt) == 0) {
        printf("OTP Verified\n");
    } else {
        printf("OTP Incorrect\n");
    }

    // Remove message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}