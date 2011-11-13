#include <stdio.h>

double find(int total, int n)
{
	int number, score;
	double average;
	scanf("%d", &number);
	if (number!=0) {
		scanf("%d", &score);
		average = find(total+score, n+1);
		if (score >= average)
			printf("%d:%d\n", number, score);
		return average;
	} else {
		average = total*1.0/n;
		printf("Average=%f\n", average);
		return average;
	}
}

int main()
{
	find(0, 0);
}