#include <stdio.h>

int fibonacci_recursively(int n)
{
	if (n==0 || n==1)
		return n;
	else
		return fibonacci_recursively(n-1) + fibonacci_recursively(n-2);
}


int fibonacci_recurrencely(int n)
{
	static int arr[100];
	if (n==0 || n==1)
		return n;
	if (arr[n]==0) 
		arr[n] = fibonacci_recurrencely(n-1) + fibonacci_recurrencely(n-2);
	return arr[n];
}

void main()
{
	int N;
	scanf("%d", &N);
	printf("Fibonacci of %d is %d\n", N, fibonacci_recursively(N));
	printf("Fibonacci of %d is %d\n", N, fibonacci_recurrencely(N));
}