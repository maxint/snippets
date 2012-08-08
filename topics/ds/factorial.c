#include <stdio.h>

int
fac_tail_zero_1(int N)
{
    int i, j, ret;
    ret = 0;
    for (i = 2; i <= N; ++i) {
        j = i;
        while (j && j % 5 == 0)
        {
            ++ret;
            j /= 5;
        }
    }
    return ret;
}

int
fac_tail_zero_2(int N)
{
   int ret = 0;
   while (N) {
       ret += N/5;
       N /= 5;
   }
   return ret;
};

int main(int argc, char* argv[])
{
    int i, j, N, fac;
    scanf("%d", &N);
    fac = 1;
    for (i = 2; i <= N; ++i) {
        fac *= i;
    }
    printf("factorial of %d: %d\n", N, fac);
    printf("1: number of zero at the end: %d\n", fac_tail_zero_1(N));
    printf("2: number of zero at the end: %d\n", fac_tail_zero_2(N));

    return 0;
}
