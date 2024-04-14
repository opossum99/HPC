#include <iostream>
#include <vector>
#include <cmath>
#include <utility>
#include <omp.h>
#include <numeric>


using namespace std;

auto least_squares(const vector<double>& X, const vector<double>& y) {
    int n = X.size(); // Number of samples
    omp_set_dynamic(0);
    omp_set_num_threads(10);

    double start, end; 
    double x2 = 0.0;
    double xy = 0.0;
    double sm_x = 0.0;
    double sm_y = 0.0;


    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:x2,xy,sm_x,sm_y)
    for(int i = 0; i < n; i++){
	//printf("Thread = %d\n", omp_get_thread_num());
	x2 += X[i]*X[i];
	xy += X[i]*y[i];
	sm_x += X[i];
	sm_y += y[i];
    }
    end = omp_get_wtime();
    cout << "Time = " << end - start << endl;

    vector<double> ab = {0.0, 0.0};
    ab[0] = (n * xy - sm_x*sm_y)/(n*x2 - sm_x*sm_x);
    ab[1] = (sm_y - ab[0]*sm_x)/n;
    
    
    return ab;
}

int main() {
    int N = 100;
    vector<double> X(N);
    iota(X.begin(), X.end(), 1);
    vector<double> y(N);
    iota(y.begin(), y.end(), 2);



    // y = aX + b
    vector<double> ab = least_squares(X, y);
    
    cout << ab[0] << "     " << ab[1] << endl;

    return 0;
}

