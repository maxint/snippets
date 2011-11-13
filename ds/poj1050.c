#include <stdio.h>

int main()
{
    int N;
    int d[100][100];
    int a[100]; // i-j
    int i,j,k,res,acc;
    while ((scanf("%d", &N)==1) && (N>0))
    {
        for (i=0; i<N; ++i) for (j=0; j<N; ++j) 
            scanf("%d", &d[i][j]);

        res=d[0][0];
        for (i=0; i<N; ++i)
        {
            memset(a,0,sizeof(a));
            for (j=i; j<N; ++j)
            {
                for (k=0; k<N; ++k)
                    a[k]+=d[j][k];
                acc=0;
                for (k=0; k<N; ++k)
                {
                    if (acc>0)
                        acc+=a[k];
                    else
                        acc=a[k];
                    if (acc>res)
                        res=acc;
                }
            }
        }
        printf("%d", res);
    }
    return 0;
}