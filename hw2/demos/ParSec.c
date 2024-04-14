#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    const size_t N = 20;

    int i, tid;

    float a[N], b[N], c[N], d[N];

    #pragma omp parallel
    {
        #pragma omp for
        for (i = 0; i < N; ++i)
        {
            a[i] = i;
            b[i] = N - i;
        }
    }

    #pragma omp parallel shared(a,b,c,d) private(i)
    {
	tid = omp_get_thread_num();
        #pragma omp sections nowait
        {
	    #pragma omp section
            for (i = 0; i < N; ++i)
            {
		printf("1st section: %d\n", omp_get_thread_num());
                c[i] = a[i] + b[i];
            }
            #pragma omp section
            for (i = 0; i < N; ++i)
            {
		printf("2nd section: %d\n", omp_get_thread_num());
                d[i] = a[i] * b[i];
            }
        }
	printf("thread == %d\n", tid);
    }

    for (i = 0; i < N; ++i)
    {
        printf("c[%1$d] = %2$f, d[%1$d] = %3$f\n", i, c[i], d[i]); 
    }

    return 0;
 }
