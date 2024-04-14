#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    /* Uncomment the code below to manually set the number of threads */
    omp_set_dynamic(0);
    omp_set_num_threads(10);

    int nthreads, tid, ndynamic;

    tid = omp_get_thread_num();
    nthreads = omp_get_num_threads();
    #pragma omp parallel private(nthreads, tid)
    {
        //tid = omp_get_thread_num();
        printf("Hello World from thread = %d\n", tid);

        if (tid == 0)
        {
            //nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);
        }
    }

    printf("Another variant2:\n\n\n\n");

    omp_set_num_threads(12);
    #pragma omp parallel private(nthreads, tid)
    {
        tid = omp_get_thread_num();
        printf("Hello World_2 from thread = %d\n", tid);

        if (tid == 0)
        {
            nthreads = omp_get_num_threads();
            printf("Number of threads_2 = %d\n", nthreads);
        }
    }

    return 0;
}
