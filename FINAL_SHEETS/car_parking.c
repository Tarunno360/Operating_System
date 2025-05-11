#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int t_id[5]={1,2,3,4,5};
void *car_park(int *id);
int spot = 1;


sem_t s;
int main(){
	pthread_t cars[5];
	sem_init(&s,0,1);

	for(int i=0;i<5;i++){
		pthread_create(&cars[i],NULL,(void *)car_park,&t_id[i]);
	}
	
	
	for(int i=0;i<4;i++){
		pthread_join(cars[i],NULL);
	}
	sem_destroy(&s);

	
	return 0;
}
void *car_park(int *id){

	if (sem_trywait(&s) != 0) {
        	printf("Car %d is waiting....\n", *id);
		sem_wait(&s); 
	}
	printf("Car %d entered. Available spots: 0\n", *id);
	sleep(0.1);
	printf("Car %d exited. Available spots: 1\n", *id);
	sem_post(&s);

	
	}
	

