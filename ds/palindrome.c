#include <stdio.h>

int find(const char *str, int n) {
	if (n<=1) 
		return 1;
	else if (str[0]==str[n-1]) 
		return find(str+1, n-2);
	else
		return 0;
}

int main()
{
	char *str = "abcdeedcba";
	printf("%s: %s\n", str, find(str, strlen(str)) ? "Yes" : "No");
}