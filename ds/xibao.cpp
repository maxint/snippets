#include <iostream>

using namespace std;

const int dx[] = {-1, 0, 1, 0};
const int dy[] = {0, 1, 0, -1};

#define MAX_BZ 100
int arr[MAX_BZ][MAX_BZ];
bool bz[MAX_BZ][MAX_BZ];
int sum = 0;
int h_stack[MAX_BZ*MAX_BZ][2];

void doing(int p, int q, int w, int h)
{
	++sum;
	bz[p][q] = false;
	h_stack[0][0] = p;
	h_stack[0][1] = q;
	int h_top = 0;
	do
	{
		int cur_x = h_stack[h_top][0];
		int cur_y = h_stack[h_top][1];
		--h_top;
		for (int i=0; i<4; ++i)
		{
			int x = cur_x + dx[i];
			int y = cur_y + dy[i];
			if (x>=0 && y>=0 && x<h && y<w && bz[x][y])
			{
				++h_top;
				h_stack[h_top][0] = x;
				h_stack[h_top][1] = y;
				bz[x][y] = false;
			}
		}
	} while(h_top>=0);
}

void main()
{
	int R, C;
	char ch;
	freopen("xibao_in.txt", "rt", stdin);
	cin >> R >> C;
	for (int i=0; i<R; ++i)
	{
		for (int j=0; j<C; ++j)
		{
			cin >> ch;
			arr[i][j] = ch - '0';
			bz[i][j] = (arr[i][j] != 0);
		}
	}
	for (int i=0; i<R; ++i)	for (int j=0; j<C; ++j) if (bz[i][j])
		doing(i, j, C, R);
	cout << sum << endl;
}