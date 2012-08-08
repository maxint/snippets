#include <foo.h>

int main(int argc, char* argv[])
{
	add(1,2);
	return 0;
}

int sum(int n)
{
	int t = 0;
	for (int i=0; i<n; ++i)
		t += add(i,t);

	return t;
}