#include <stdio.h>


int fibonacci(int n){
	int res;
	if(n == 0){
		return 0;
	}
	else if(n == 1 || n == 2){
		return 1;
	}	
	
	else{
		res = fibonacci(n-1) + fibonacci(n-2);
	}
	
}









int main()
{
	int result = fibonacci(8);
	printf("%d", result);

	return 0;
}

