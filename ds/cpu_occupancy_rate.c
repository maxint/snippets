#include <stdio.h>

int main(int argc, char* argv[])
{
    int i;
    for (;;)
    {
        for (i = 0; i < 96000000; ++i) ;
        sleep(10);
    }
    return 0;
}
