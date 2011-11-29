#include <stdio.h>

const double ABS_EPS = 1E-8;

double ABS(double a)
{
	return (a > 0 ? a : -a);
}

void main()
{
	double x0 = 0;
	double x1 = 1 / (x0+2);
	while (ABS(x1-x0) > ABS_EPS)
	{
		x0 = x1;
		x1 = 1 / (x0+2);
	}
	x1 += 1;
	printf("sqrt(2) = %.9e\n", x1);
}