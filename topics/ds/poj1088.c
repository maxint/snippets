#include <stdio.h>

int arr[102][102];
short len[102][102];
int r, c;
#define MAX_HEGHT 0x7fffffff

int get_max_len(int y, int x);

void main()
{
	int i, j;
	int max_len = 0;
	scanf("%d %d", &r, &c);
	for (i=0; i<r+2; ++i) {
		arr[r+1][i] = arr[0][i] = MAX_HEGHT;
		arr[i][0] = arr[i][c+1] = MAX_HEGHT;
	}
	for (i=0; i<r; ++i)
		for (j=0; j<c; ++j)
			scanf("%d", &arr[i+1][j+1]);
	memset(len, 0, sizeof(len));
	for (i=1; i<=r; ++i) {
		for (j=1; j<=c; ++j) {
			len[i][j] = get_max_len(i,j);
			if (max_len < len[i][j])
				max_len = len[i][j];
		}
	}
	printf("%d", max_len+1);
}

int off[4][2] = { 0, -1, -1, 0, 0, 1, 1, 0 };

int get_max_len(int y, int x)
{
	int i;
	int tmax = 0;
	if (len[y][x]!=0)
		return len[y][x];
	for (i=0; i<4; ++i) {
		int yy = y+off[i][0];
		int xx = x+off[i][1];
		if (arr[yy][xx]<arr[y][x])
		{
			int tres = get_max_len(yy, xx)+1;
			if (tmax<tres)
				tmax = tres;
		}
	}
	return tmax;
}