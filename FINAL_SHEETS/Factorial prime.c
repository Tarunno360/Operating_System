#include <sys/wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
void main()
{	FILE *fp;
	FILE *fp2;
	int a,b, status;
	int user;
	scanf("%d", &user);
	a = fork();
	char prime[256];
	char factor[256];
	if(a >0){
		wait(&status);
		b = fork();
		if(b>0){
			wait(&status);
			fp = fopen("factorial.txt", "r"); 
			fp2 = fopen("prime.txt", "r");
			fgets(factor, sizeof(factor), fp);
			fgets(prime, sizeof(prime), fp2);
			printf("Factorial: %s", factor);
			printf("Prime: %s", prime);
			
		}
		else if(b == 0){
			fp2 = fopen("prime.txt", "w");
			for(int i = 1; i <= user; i++){
				bool prime = true; 
				for(int j = 2; j < i/2; j++){
					if (i % j == 0){
						prime = false;
				
					}
					
					
				} 
				if ( prime == true){
					
					fprintf(fp2, "%d, ", i);
					
					
				}
				
				
			}  
			fclose(fp2);
		}
		
	}
	else if(a==0){
		
		int fac_res = user;
		for(int i = user-1; i > 0; i--){
			fac_res *= i;
		}
		fp = fopen("factorial.txt", "w");
		fprintf(fp, "%d", fac_res);
		fclose(fp);
	}
	
}


