#include <stdio.h>
#include <stdlib.h>

char *MaxSubString(char *str1, char *str2) {
	int i, j, k, index, max=0;
	for(i=0; str1[i]; i++)
		for(j=0; str2[j]; j++) {
			for(k=0; str1[i+k]==str2[j+k] && (str2[i+k] || str1[i+k]); k++);
			if(k>max) {		// 出现大于当前子串长度的子串，则替换子串位置和程度
				index = j;	max = k;
			}
		}
	char *strResult = (char *)calloc(sizeof(char), max+1);
	for(i=0; i<max; i++)		
		strResult[i] = str2[index++];
	return strResult;
}

int main(int argc, char* argv[]) {
	char str1[] = "abractyeffyt", str2[] = "dgdsaeactffyey";
	char *strResult = MaxSubString(str1, str2);
	printf("str1=%s\nstr2=%s\nMaxSubString=%s\n", str1, str2, strResult);
}
