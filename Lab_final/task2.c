#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int main()
{	FILE *fp;
	int a,b,p, status;
	int user,sum;
    printf("Enter an integar:");
	scanf("%d", &user);
	b = fork();
    //p=getpid(b);
	char factor[256];
	if(b>0){
			wait(&status);
			fp = fopen("22101576_Task3.txt", "r"); 
			fgets(factor, sizeof(factor), fp);
			printf("Factorial: %s", factor);
	}		
	else if(b==0 && getpid()%2==0){
        printf("Child PID is even\n");
		fp = fopen("22101576_Task3.txt", "w");
		int fac_res = user;
        printf("Factorial up to %d\n",user);
		for (int i = user-1; i > 0; i--){
			fac_res *= i;
            printf("%d ",fac_res);
            fprintf(fp, "%d,", fac_res);
		}
		fprintf(fp, "%d", fac_res);
		fclose(fp);
	}
    else if(b==0 && getpid()%2!=0){
        a=fork();
        fp = fopen("22101576_Task3.txt", "a");
        int sum=0;
        for (int i=1;i<user;i+=2){
            sum+=i;
        }
        fprintf(fp, "%d", sum);
        fclose(fp);
    }
	return 0;
}


