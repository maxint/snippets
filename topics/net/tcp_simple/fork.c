#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define EXIT_FALURE -1

int main( int argc, char* argv[] )
{
    pid_t pid;
    printf("begin of main\n");
    pid = fork();
    if (pid==0)
    {
        int j;
        for (j=0; j<10; ++j)
        {
            printf("child: %d\n", j);
            sleep(1);
        }
        _exit(0);
    }
    else if (pid>0)
    {
        int j;
        for (j=0; j<10; ++j)
        {
            printf("parent: %d\n", j);
            sleep(1);
        }
        _exit(0);
    }
    else
    {
        fprintf ( stderr, "can't fork, error %d\n", errno );
        exit(EXIT_FALURE);
    }

    return 0;
}

