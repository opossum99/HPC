#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

float dotprod(float * a, float * b, size_t N)
{
    int i, tid;
    float sum;

    tid = omp_get_thread_num();

    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < N; ++i)
    {
        sum += a[i] * b[i];
        printf("tid = %d i = %d\n", tid, i);
    }

    return sum;
}

int main (int argc, char *argv[])
{
    const size_t N = 100;
    int i;
    float sum;
    float a[N], b[N];
    double start, end;


    for (i = 0; i < N; ++i)
    {
        a[i] = b[i] = (double)i;
	b[i] = 1.;
    }

    start = omp_get_wtime();

    #pragma omp parallel
    sum = dotprod(a, b, N);

    end = omp_get_wtime();

    printf("Time = %f\n", end - start);
    printf("Sum = %f\n",sum);

    return 0;
}
