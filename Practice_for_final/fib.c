#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
int n,answer;
int fibonacci(int n) {
    if (n==0) return 0;
    else if (n==1 || n==2) return 1;
    else{
        return fibonacci(n - 1)+fibonacci(n - 2);
    }
}
int main(){
    scanf("%d",&n);
    answer=fibonacci(n);
    printf("The %dth Fibonacci number is %d\n", n, answer);
    return 0;
}