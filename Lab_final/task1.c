#include <stdio.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
//#include <pthread.h>
//#include <semaphore.h>

int fibonacci(int n){
	int res;
	if(n == 0){
        //printf("0 ");
		return 0;
        //printf("0 ");
	}
	else if(n == 1 || n == 2){
        //printf("1 ");
		return 1;
        //printf("1 ");
	}	
	else{
        //printf("%d ",res);
		return fibonacci(n-1) + fibonacci(n-2);
        //printf("%d ",res);
	}
}
int main()
{   
    int n;
    printf("Please enter your value:");
    scanf("%d",&n);
	//int result = fibonacci(8);
	//printf("%d", result);
    for (int i = 0; i < n; i++){
        printf("%d ", fibonacci(i));
    }
	return 0;
}