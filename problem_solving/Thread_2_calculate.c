#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
void *func_thread(int *n);
void *t_ret;
int arr[4]={3,8,5,2};
void *func_thread2(int *n);
void *t_ret2;
int main(){
	pthread_t t1;
	pthread_create(&t1,NULL,(void *)func_thread,arr);
	pthread_join(t1,&t_ret);
	//printf("Thread returned: %d\n",(int *)t_ret);
	//sleep(10);
	pthread_t t2;
	pthread_create(&t2,NULL,(void *)func_thread2,arr);
	pthread_join(t2,&t_ret2);
	return 0;
}

void *func_thread(int *n){
	printf("Thread1:\n");
	for (int i=0; i<4 ; i++){
	n[i]=2*(n[i] * n[i]);
	printf(" %d",n[i]);
	}
}
void *func_thread2(int *n){
	printf("Thread2:\n");
	for (int i=0; i<4 ; i++){
	n[i]= n[i]+7;
	printf(" %d",n[i]);
	}
}
//restoring 2