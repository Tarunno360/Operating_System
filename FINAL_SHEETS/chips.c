#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int t_id[5]={1,2,3,4,5};
void *customer_buy(int *id);
int chips = 5;

pthread_mutex_t mutex;

int main(){
	pthread_t customers[5];

	pthread_mutex_init(&mutex,NULL);
	for(int i=0;i<5;i++){
		pthread_create(&customers[i],NULL,(void *)customer_buy,&t_id[i]);
	}
	
	
	for(int i=0;i<4;i++){
		pthread_join(customers[i],NULL);
	}
	pthread_mutex_destroy(&mutex);

	
	return 0;
}
void *customer_buy(int *id){

	pthread_mutex_lock(&mutex);
	
	if (chips>0){
		chips--;
		printf("Customer %d purchased Chips\n", *id);
		printf("Remaining stock: %d\n", chips);
	}
	pthread_mutex_unlock(&mutex);
	
	}
	

