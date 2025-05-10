#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
int main(){
    FILE *fp_w1,*fp_w2,*fp_r1,*fp_r2,status;
	int pid,pid2,i,j,inp1,answer1=1,k;
    scanf("%d",&inp1);
    fp_w1= fopen("factorial.txt","w");
    fp_w2= fopen("primes.txt","w");
	pid= fork();
	if (pid==0){
	    printf("Im child process1\n");
        for (i=inp1;i>=1;i--){
            answer1*=i;
            }
        fprintf(fp_w1,"The factorial of %d is %d\n", inp1, answer1);
        //fclose(fp_w1);
        }
    else{
        printf("Im parent process\n");
        wait(NULL);
        pid2= fork();
        if(pid2==0){
	        printf("Im child process2\n");
            for (j=inp1;j>=1;j--){
                for (k=j-1;k>=1;k--) {
                    if (k==1){
                        fprintf(fp_w2,"%d ",j);
                        }
                    else if (j%k==0){
                        break;}
                    }
                }
                exit(0);
                }
            
        }
        wait(NULL);
        fp_r1= fopen("factorial.txt","r");
        fp_r2= fopen("primes.txt","r");
        printf("\nContents of factorial.txt:\n");
        char line[256];  
        while (fgets(line, sizeof(line), fp_r1)) {
            printf("%s", line);  
            }
        printf("\nContents of primes.txt:\n");
        while (fgets(line, sizeof(line), fp_r2)) {
            printf("%s", line);
        }
        
        fclose(fp_w1);
        fclose(fp_w2);
        fclose(fp_r1);
        fclose(fp_r2);
        return 0;
    }

