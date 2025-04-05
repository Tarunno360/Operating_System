#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    int a, b, c;

    key_t key = ftok("shmfile", 65); 
    int shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT); 
    int *total_process_count = (int *)shmat(shmid, NULL, 0); 
    *total_process_count = 1;


    a = fork();
    b = fork();
    c = fork();
    if (a == 0) {
        *total_process_count += 1;
        if (getpid() % 2 != 0) {
            fork();
            *total_process_count+=1;
            
        }
    } else if (b == 0) {
        *total_process_count += 1;
        if (getpid() % 2 != 0) {
            fork();
            *total_process_count+=1;
            
        }
    } else if (c == 0) {
        *total_process_count += 1;
        if (getpid() % 2 != 0) {
            fork();
            *total_process_count+=1;
        }

    } else if (a > 0 && b > 0 && c > 0) {
        while (wait(NULL) > 0);
        printf("Total number of processes created: %d\n", *total_process_count);

        shmdt(total_process_count);
        shmctl(shmid, IPC_RMID, NULL);

    }
    return 0;
}