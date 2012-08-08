#include <iostream>

using namespace std;

union {
	int i;
	char ch[4];
} a;

int main()
{
	a.ch[0] = 1;
	a.ch[1] = 10;
	//a.ch[2] = 1;
	cout << a.i << endl;
	cout << (a.ch[0] + a.ch[1]*256 + a.ch[2]*256*256) << endl;
}