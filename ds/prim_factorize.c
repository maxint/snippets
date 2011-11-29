#include <stdio.h>

void prim(int m, int n);

void main()
{
	int n = 435234;
	printf("%d=", n);
	prim(n, 2);
}

void prim(int m, int n)
{
	if (m>n) {
		while (m%n != 0) ++n;
		m /= n;
		prim(m, n);
		printf("%d*", n);
	}
}