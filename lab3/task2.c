#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

struct my_msg {
    long int type;
    char txt[6];
};

int main() {
    key_t key;
    int msgid;
    struct my_msg message;
    pid_t pid_otp, pid_mail;
    char workspace[10];

    key = ftok(".", 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    printf("Please enter the workspace name:\n");
    scanf("%s", workspace);

    if (strcmp(workspace, "cse321") != 0) {
        printf("Invalid workspace name\n");
        msgctl(msgid, IPC_RMID, NULL);
        exit(0);
    }

    message.type = 1; 
    strncpy(message.txt, "cse321", 6);
    if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("Workspace name sent to otp generator from log in: %s\n", message.txt);

    pid_otp = fork();
    if (pid_otp < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_otp == 0) { 
        if (msgrcv(msgid, &message, sizeof(message.txt), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        printf("\nOTP generator received workspace name from log in: %s\n", message.txt);

        int otp = getpid();
        snprintf(message.txt, 6, "%d", otp);
        message.type = 2;
        if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        printf("OTP sent to log in from OTP generator: %s\n", message.txt);

        message.type = 3;
        if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        printf("OTP sent to mail from OTP generator: %s\n", message.txt);

        pid_mail = fork();
        if (pid_mail < 0) {
            perror("fork");
            exit(1);
        }

        if (pid_mail == 0) { 
            if (msgrcv(msgid, &message, sizeof(message.txt), 3, 0) == -1) {
                perror("msgrcv");
                exit(1);
            }
            printf("Mail received OTP from OTP generator: %s\n", message.txt);

            message.type = 4; 
            if (msgsnd(msgid, &message, sizeof(message.txt), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
            printf("OTP sent to log in from mail: %s\n", message.txt);

            exit(0);
        }

        wait(NULL);

        exit(0);
    }

    wait(NULL);

    if (msgrcv(msgid, &message, sizeof(message.txt), 2, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    char otp_from_generator[6];
    strncpy(otp_from_generator, message.txt, 6);
    printf("Log in received OTP from OTP generator: %s\n", otp_from_generator);

    if (msgrcv(msgid, &message, sizeof(message.txt), 4, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    printf("Log in received OTP from mail: %s\n", message.txt);

    if (strcmp(otp_from_generator, message.txt) == 0) {
        printf("OTP Verified\n");
    } else {
        printf("OTP Incorrect\n");
    }

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    return 0;
}