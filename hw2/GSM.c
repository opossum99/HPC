#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void GSM(double* A, double* x, double *b, int N){
    double sum = 0.0;
    int i = 0;
    int j = 0;
    for(i = 0; i < N; i++){
	sum = 0.0;
        #pragma omp parallel for reduction(+:sum)
        for(j = i+1; j < N; j++){
	    //printf("Thread = %d", omp_get_thread_num());
            sum += A[i*N + j]*x[j];
	}
        #pragma omp parallel for reduction(+:sum)
	for(j = 0; j < i; j++){
	    sum += A[i*N + j]*x[j];
	}
        x[i] = (b[i] - sum)/A[i*N + i];
    }    
}


void create(double* A, double *x, double* b){
    A[0] = 10.;
    A[1] = -1.;
    A[2] = 2.;
    A[3] = 0.;
    A[4] = -1.;
    A[5] = 11.;
    A[6] = -1.;
    A[7] = 3.;
    A[8] = 2.;
    A[9] = -1.;
    A[10] = 10.;
    A[11] = -1.;
    A[12] = 0.;
    A[13] = 3.;
    A[14] = -1.;
    A[15] = 8.;

    b[0] = 6.;
    b[1] = 25.;
    b[2] = -11.;
    b[3] = 15.;

    x[0] = 0.;
    x[1] = 0.;
    x[2] = 0.;
    x[3] = 0.;
}


int main(){
    int max_it = 10;
    int i = 0;
    int N = 4;
    double *A, *b, *x;
    A = new double[N*N];
    b = new double[N];
    x = new double[N];
    create(A, x, b);
    for(int i = 0; i < N; i++){
	printf("b[%d] = %f\n", i, b[i]);
    }
    for(int i = 0; i < N; i++){
	printf("x_0[%d] = %f\n", i, x[i]);
    }
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%f  ", A[i*N + j]);
	}
	printf("\n");
    }
    printf("\n\n\n");



    for(i = 0; i < max_it; i++){
        GSM(A, x, b, N);
	printf("Iterations: %d\n", i);
	for(int i = 0; i < N; i++){
	    printf("x_0[%d] = %f\n", i, x[i]);
        }
	printf("\n");
    }
    for(int i = 0; i < N; i++){
	printf("x[%d] = %f\n", i, x[i]);
    }

    
    delete[] A;
    delete[] b;
    delete[] x;
}
