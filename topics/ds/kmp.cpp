#include <iostream>
#include <ctime>

using namespace std;

void kmp(const char* s1, const char* s2);

int main()
{
	char *str1 = "abababaababacb";
	char *str2 = "ababacb";
	kmp(str1, str2);
}

void kmp(const char* s1, const char* s2)
{
	int n = strlen(s1);
	int m = strlen(s2);

	// precompute
	int *p = new int[m];

	clock_t tt = clock();
	p[0] = -1;
	int j = -1;
	for (int i=1; i<m; ++i)
	{
		while (j>-1 && s2[j+1]!=s2[i]) j = p[j];
		if (s2[j+1] == s2[i]) ++j;
		p[i] = j;
	}

	// process
	j = -1;
	for (int i=0; i<n; ++i)
	{
		while (j>-1 && s2[j+1]!=s1[i]) j = p[j];
		if (s2[j+1] == s1[i]) ++j;
		if (j == m-1)
		{
			cout << "Pattern occurs with shift " << i-m-1 << endl;
			j = p[j];
		}
	}
	cout << "Finished in " << clock()-tt << "ms" << endl;
	delete []p;
}