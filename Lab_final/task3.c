#include <stdio.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int t_id[3]={1,2,3};
void *seat_booking(int *id);
int seats=1;

pthread_mutex_t mutex;

int main(){
    pthread_t customers[3];
    pthread_mutex_init(&mutex,NULL);
    for (int i=0;i<3;i++){
        pthread_create(&customers[i],NULL,(void*)seat_booking,&t_id[i]);
    }
    for (int i=0;i<3;i++){
        pthread_join(&customers[i],NULL);
    }
    pthread_mutex_destroy(&mutex);
    return 0;

}
void *seat_booking(int *id){
    pthread_mutex_lock(&mutex);
    if (seats<4){
        seats++;
        printf("passenger %d booked seat number %d \n",*id,seats);
        int seat_rem=3-seats;
        printf("Seat remaining %d",seat_rem);
    }
    else{
        printf('No seats');
    }
    pthread_mutex_unlock(&mutex);
}
