#include <stdio.h>
#include <memory.h>

#define N 8

unsigned char row[N], col[N], diag1[2*N], diag2[2*N];

void pr()
{
	static c=0;
	int i;
	printf("#%3d:\t", ++c);
	for (i=0; i<N; ++i)
		printf("%d ", row[i]);
	printf("\n");
}

void try_step(int t)
{
	int i;
	for (i=0; i<N; ++i)
	{
		if (col[i] && diag1[t+i] && diag2[t-i+N-1])
		{
			row[t] = i;
			col[i] = 0;
			diag1[t+i] = 0;
			diag2[t-i+N-1] = 0;
			if (t==N-1) pr();
			else try_step(t+1);
			col[i] = 1;
			diag1[t+i] = 1;
			diag2[t-i+N-1] = 1;
		}
	}
}

void main()
{
	memset(col, 1, sizeof(col));
	memset(diag1, 1, sizeof(diag1));
	memset(diag2, 1, sizeof(diag2));
	try_step(0);
}