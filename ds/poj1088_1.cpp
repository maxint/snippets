#include <iostream>
#include <algorithm>

using namespace std;

typedef short uchar;

int arr[100][100];
int len[100][100];

struct dot
{
	uchar x;
	uchar y;
	int h;
};

dot line[10000];

int cmp(const void *arg1, const void* arg2)
{
	return (((dot*)arg1)->h>((dot*)arg2)->h) ? 1 : -1;
}

int off[4][2] = { 0, -1, -1, 0, 0, 1, 1, 0 };

int main()
{
	uchar r, c;
	cin >> r >> c;
	int num=0;
	for (uchar i=0; i<r; ++i) 
	{
		for (uchar j=0; j<c; ++j)
		{
			cin >> arr[i][j];
			line[num].y = i;
			line[num].x = j;
			line[num].h = arr[i][j];
			++num;
		}
	}
	qsort(line, r*c, sizeof(line[0]), cmp);
	for (int i=0; i<r*c; ++i)
	{
		for (int k=0; k<4; ++k)
		{
			int yy = line[i].y+off[k][0];
			int xx = line[i].x+off[k][1];
			if (yy>=0 && xx>=0 && yy<r && xx<c &&
				arr[yy][xx]<arr[line[i].y][line[i].x] &&
				len[yy][xx]>=len[line[i].y][line[i].x])
			{
				len[line[i].y][line[i].x] = len[yy][xx]+1;
			}
		}
	}
	int max_len = len[0][0];
	for (int i=0; i<r; ++i)
	{
		for (int j=0; j<c; ++j)
		{
			if (len[i][j]>max_len)
				max_len = len[i][j];
		}
	}
	cout << max_len+1;
}