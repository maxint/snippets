#include <omp.h>
#include <stdio.h>

void main(int argc, char *argv)
{
#pragma omp parallel sections num_threads(4)
	{
#pragma omp section
		printf("section 1 ThreadId = %d\n", omp_get_thread_num());
#pragma omp section 
		printf("section 2 ThreadId = %d\n", omp_get_thread_num());
#pragma omp section
		printf("section 3 ThreadId = %d\n", omp_get_thread_num());
#pragma omp section 
		printf("section 4 ThreadId = %d\n", omp_get_thread_num());
	}
}