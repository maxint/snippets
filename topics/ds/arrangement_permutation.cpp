#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

using namespace std;

template <class BidItor>
bool next_permutation_my(BidItor first, BidItor last)
{
	if (first == last || first == --last) return false;
	BidItor i = last;
	for (;;)
	{
		BidItor ii = i;
		--i;
		if (*i < *ii)
		{
			BidItor j = last;
			while (!(*i < *j)) --j;
			swap(*i, *j);
			reverse(ii, last+1);
			return true;
		}
		if (i == first)
		{
			reverse(first, last+1);
			return false;
		}
	}
}

void combine(int a[], int n, int m, int b[], int M)
{
	for (int i=m; i<=n; ++i)
	{
		b[m-1] = i-1;
		if (m>1)
			combine(a, i-1, m-1, b, M);
		else
		{
			for (int j=0; j<M; ++j)
			{
				cout << a[b[j]] << " ";
			}
			cout << endl;
		}
	}
}

void combine2(int a[], int n, int m)
{
	int *order = new int[m];
	int c = 0;
	int depth = m-1;
	for (int i=0; i<m; ++i)
		order[i] = i;
	while(depth>=0)
	{
		if (depth == m-1)
		{
			++c;
			cout << c << ": ";
			for (int i=0; i<m; ++i)
				cout << a[order[i]] << " ";
			cout << endl;
			++order[depth];
			if (order[depth] < n)
				continue;
			else
			{
				--depth;
				++order[depth];
			}
		}
		if (order[depth] < n && 
			n - order[depth] >= m - depth)
		{
			++depth;
			order[depth] = order[depth-1]+1;
		}
		else
		{
			--depth;
			++order[depth];
		}
	}
}

int main()
{
#if 0
	int ia[] = {1,2,3,4};
	vector<int> iv(ia,ia+sizeof(ia)/sizeof(int));

	copy(iv.begin(),iv.end(),ostream_iterator<int>(cout," "));
	cout << endl;
	while(next_permutation_my(iv.begin(),iv.end()))
	{
		copy(iv.begin(),iv.end(),ostream_iterator<int>(cout," "));
		cout << endl;
	}
#else
	const int N = 4;
	const int M = 3;
	int a[N];
	int b[M];
	for (int i=0; i<N; ++i)
		a[i] = i+1;
	//combine(a, N, M, b, M);
	combine2(a, N, M);
#endif

	return 0;
}