#include <stdio.h>

#define MAX 30

int
max_k(int *arr, int N)
{
    int i, vmax, vmin;
    vmax = N; vmin=0;
    while (vmax - vmin > 0.5) {
        vmid = vmin + (vmax-vmin)/2;
    }
};

int main(int argc, char* argv[])
{
    int arr[MAX];
    int i;
    for (i = 0; i < MAX; ++i) {
        arr[i] = rand() % MAX*2;
    }

    return 0;
}
