#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <numbers>\n", argv[0]);
        return 1;
    }

    pid_t pid_sort, pid_oddeven;

    pid_sort = fork();

    if (pid_sort == 0) { 

        execvp("./sort", argv);  

    } else if (pid_sort > 0) { 
        wait(NULL);  

        pid_oddeven = fork();

        if (pid_oddeven == 0) {  

            execvp("./oddeven", argv);  

        } else if (pid_oddeven > 0) {
            wait(NULL);  
        } else {
            printf("fork failed for oddeven.c");
            exit(1);
        }
    } else {
        printf("fork failed for sort.c");
        exit(1);
    }

    return 0;
}
