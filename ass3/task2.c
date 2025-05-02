#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

// Define the message structure
struct msgbuf {
    long mtype; 
    char txt[6];
};

int validate_workspace(const char *workspace) {
    return strcmp(workspace, "cse321") == 0;
}

int main() {
    key_t key;
    int msgid;
    struct msgbuf message;
    pid_t otp_pid, mail_pid;
    char workspace[100];

    // Generate a unique key for the message queue
    key = ftok(".", 'A');
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }

    // Create a message queue
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        exit(1);
    }

    // Get workspace name from user
    printf("Please enter the workspace name:\n");
    scanf("%s", workspace);

    if (!validate_workspace(workspace)) {
        printf("Invalid workspace name\n");
        // Remove the message queue before exiting
        msgctl(msgid, IPC_RMID, NULL);
        exit(0);
    }

    // Login process: Send workspace name to OTP generator
    message.mtype = 1; // Type for OTP generator
    strncpy(message.txt, workspace, 5);
    message.txt[5] = '\0'; // Ensure null termination
    if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
        perror("msgsnd failed");
        exit(1);
    }
    printf("Workspace name sent to otp generator from log in: %s\n", message.txt);

    // Fork to create OTP generator process
    otp_pid = fork();
    if (otp_pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (otp_pid == 0) { // OTP generator process
        // Read workspace name from login
        if (msgrcv(msgid, &message, sizeof(message.txt), 1, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }
        printf("\nOTP generator received workspace name from log in: %s\n", message.txt);

        // Generate OTP (process ID)
        pid_t otp = getpid();
        snprintf(message.txt, 6, "%d", otp);
        message.txt[5] = '\0'; // Ensure null termination

        // Send OTP to login
        message.mtype = 2; // Type for login
        if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
            perror("msgsnd failed");
            exit(1);
        }
        printf("\nOTP sent to log in from OTP generator: %s\n", message.txt);

        // Send OTP to mail
        message.mtype = 3; // Type for mail
        if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
            perror("msgsnd failed");
            exit(1);
        }
        printf("OTP sent to mail from OTP generator: %s\n", message.txt);

        // Fork to create mail process
        mail_pid = fork();
        if (mail_pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (mail_pid == 0) { // Mail process
            // Read OTP from OTP generator
            if (msgrcv(msgid, &message, sizeof(message.txt), 3, 0) == -1) {
                perror("msgrcv failed");
                exit(1);
            }
            printf("\nMail received OTP from OTP generator: %s\n", message.txt);

            // Send OTP back to login
            message.mtype = 4; // Type for login from mail
            if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
                perror("msgsnd failed");
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

    // Login waits for OTP generator to terminate
    wait(NULL);

    // Login process: Receive OTP from OTP generator
    if (msgrcv(msgid, &message, sizeof(message.txt), 2, 0) == -1) {
        perror("msgrcv failed");
        exit(1);
    }
    char otp_from_generator[6];
    strncpy(otp_from_generator, message.txt, 6);
    printf("\nLog in received OTP from OTP generator: %s\n", otp_from_generator);

    // Login process: Receive OTP from mail
    if (msgrcv(msgid, &message, sizeof(message.txt), 4, 0) == -1) {
        perror("msgrcv failed");
        exit(1);
    }
    char otp_from_mail[6];
    strncpy(otp_from_mail, message.txt, 6);
    printf("Log in received OTP from mail: %s\n", otp_from_mail);

    // Compare OTPs
    if (strcmp(otp_from_generator, otp_from_mail) == 0) {
        printf("OTP Verified\n");
    } else {
        printf("OTP Incorrect\n");
    }

    // Remove the message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl failed");
        exit(1);
    }

    return 0;
}