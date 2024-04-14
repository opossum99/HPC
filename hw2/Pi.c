#include <stdio.h>
#include <omp.h>
#include <math.h>

int main()
{
    const size_t N = 100000;
    double step;

    int tid = 0;
    double x, pi, sum = 0.;
    double start = 0., end = 0.;

    step = 1. / (double)N;
    
    start = omp_get_wtime();

    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; ++i)
    {
	tid = omp_get_thread_num();
        x = (i + 0.5) * step;
        sum += 4.0 / (1. + x * x);
	printf("tid = %d\n", tid);
    }

    end = omp_get_wtime();
    printf("time = %f\n", end - start);

    pi = step * sum;

    printf("pi = %.16f\n", pi);
    printf("pi_real = %.16f\n", M_PI);

    return 0;
}
