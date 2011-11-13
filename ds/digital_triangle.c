#include <stdio.h>

enum { LEFT, RIGHT };
void main()
{
	int N, i, j;
	int arr[100][100];
	int path[100][100];
	freopen("digital_triangle_in.txt", "r", stdin);
	scanf("%d", &N);
	for (i=0; i<N; ++i)
		for (j=0; j<=i; ++j)
			scanf("%d", &arr[i][j]);
	for (i=N-2; i>=0; --i)
	{
		for (j=0; j<=i; ++j)
		{
			arr[i][j] += (arr[i+1][j] > arr[i+1][j+1] ? arr[i+1][j] : arr[i+1][j+1]);
			path[i][j] = arr[i+1][j] > arr[i+1][j+1] ? LEFT : RIGHT;
		}
	}
	j = 0;
	printf("PATH: ");
	for (i=0; i<N-1; ++i)
	{
		printf("%d -> ", arr[i][j]);
		j += path[i][j];
	}
	printf("%d\n", arr[i][j]);
	printf("Max sum from root to leaf is %d\n", arr[0][0]);
	fclose(stdin);
}