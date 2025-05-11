#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
sem_t s;
void * writeAddress(int *id);
void * writeID(int *id);
FILE *fp;
int main(){
	
	int t_id[2]={1,2};
	system("mkdir moda");
	system("touch moda/ghum.txt");
	pthread_t t1, t2;
	sem_init(&s,0,1);
	pthread_create(&t1,NULL,(void *)writeID,&t_id[0]);
	
	pthread_create(&t2,NULL,(void *)writeAddress,&t_id[1]);
	
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	sem_destroy(&s);
	return 0;
}

void * writeID(int *id){
	sem_wait(&s);
	fp = fopen("moda/ghum.txt", "a");
	fprintf(fp, "22101349");
	fclose(fp);
	sem_post(&s);
	
}

void * writeAddress(int *id){
	sem_wait(&s);
	system("pwd >> moda/ghum.txt");
	sem_post(&s);
	
}
	
	
