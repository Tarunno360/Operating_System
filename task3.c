#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main() {
    int a, b, c;
    int total_process_count = 1;

    a = fork();
    b = fork();
    c = fork();

    if (a == 0) {
        if (getpid() % 2 != 0) {
            fork();
            total_process_count++;
        }
    } else if (b == 0) {
        if (getpid() % 2 != 0) {
            fork();
            total_process_count++;
        }
    } else if (c == 0) {

        if (getpid() % 2 != 0) {
            fork();
            total_process_count++;
        }
    } else {
        total_process_count += 3;
        printf("Total number of processes created: %d\n", total_process_count);
    }
    return 0;
}