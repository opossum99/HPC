#include <cuda_runtime.h>
#include <fstream>

namespace solver
{
  template<typename T>
  class gpu_solver
  {
    public:
      std::ofstream output;
      size_t N, M;
      T h, dt;
      T *ex, *ey, *hz, *mu, *epsilon;

      void malloc_arrays();
      void iteration(int t, bool to_save);
      gpu_solver(size_t const &N, size_t const &M, T h, T dt) : N(N), M(M), h(h), dt(dt)
      {
        malloc_arrays();
      }
      ~gpu_solver();
  };

  template<typename T>
  __global__ void initialize_source(int t, T dt, T h,
                                     T * field, size_t N, size_t M, size_t k)
  {
    int numThreads = blockDim.x * gridDim.x;
    int global_id = threadIdx.x + blockIdx.x * blockDim.x;
    for (int id = global_id; id < 2*(M-50); id+=numThreads)
    {
      int j = id % 2 + k;
      int i = id / 2 + 25;
      field[j+i*N] = sin(10*3.1416*(t*dt-j*h));
    }
    return;
  }

  template<typename T>
  __global__ void init_const(T *array, size_t N, T c)
  {
    int numThreads = blockDim.x * gridDim.x;
    int global_id = threadIdx.x + blockIdx.x * blockDim.x;
    for (int id = global_id; id < N; id+=numThreads) array[id] = c;
    return;
  }

  template<typename T>
  __global__ void init_circle(T *array, size_t N, size_t M, size_t R)
  {
    int numThreads = blockDim.x * gridDim.x;
    int global_id = threadIdx.x + blockIdx.x * blockDim.x;

    for (int id = global_id; id < N*M; id+=numThreads)
    {
      int i = id/N;
      int j = id%N;
      if ((i-M/2)*(i-M/2) + (j-N/2)*(j-N/2) < R*R)
          array[j+i*N] = 5.25;
      else
        array[j+i*N] = 1.0;
    }
    return;
  }

  __device__ double sigma(double const &r, double const &dr,
                           int const &PML, int const & size)
  {
    if (r>(dr*(size-PML))) return pow((r-(size-PML)*dr)/(PML*dr),2)*3.0*log(10.0)*13.0/(PML*dr);
    else if (r < dr*(PML)) return pow((PML*dr-r)/(PML*dr),2)*3.0*log(10.0)*13.0/(PML*dr);
    else return 0.0;
  }

  template<typename T>
  __global__ void iteration_hz(T h, T dt, T * hz, T *ex, T *ey, 
                              T *mu, T *epsilon, size_t N, size_t M)
  {
    int numThreads = blockDim.x * gridDim.x;
    int global_id = threadIdx.x + blockIdx.x * blockDim.x;

    for (int id = global_id; id < (N-2)*(M-2); id+=numThreads)
    {
      int i = id/(N-2)+1;
      int j = id%(N-2)+1;
      
      double vel_x = 1./mu[j+i*N]*(ey[j+1+i*N]-ey[j-1+i*N])/2;
      double vel_y = -1./mu[j+i*N]*(ex[j+(i+1)*N]-ex[j+(i-1)*N])/2;
      double rhs = vel_x/h+vel_y/h;
      double pml_y = 0.5*pow(1.0/2.25, 0.5)*sigma(i*h, 0.01, 10, 100);
      double pml_x = 0.5*pow(1.0/2.25, 0.5)*sigma(j*h, 0.01, 20, 200);
      hz[j+i*N] -= dt*(rhs + pml_y*hz[j+i*N] + pml_x*hz[j+i*N])/(1.0+0.5*dt*(pml_x + pml_y));
    }
    return;
  } 

  template<typename T>
  __global__ void iteration_ex(T h, T dt, T * hz, T *ex, T *ey, 
                              T *mu, T *epsilon, size_t N, size_t M)
  {
    int numThreads = blockDim.x * gridDim.x;
    int global_id = threadIdx.x + blockIdx.x * blockDim.x;

    for (int id = global_id; id < (N-2)*(M-2); id+=numThreads)
    {
      int i = id/(N-2)+1;
      int j = id%(N-2)+1;
      double vel_y = -1./epsilon[j+i*N]*(hz[j+(i+1)*N]-hz[j+(i-1)*N])/2;
      double rhs = vel_y/h;
      double pml_y = 0.5*pow(1.0/2.25, 0.5)*sigma(i*h, 0.01, 10, 100);
      ex[j+i*N] -= dt*(rhs + pml_y*ex[j+i*N])/(1.0 + 0.5*dt*pml_y);
    }
    return;
  } 

  template<typename T>
  __global__ void iteration_ey(T h, T dt, T * hz, T *ex, T *ey,
                             T *mu, T *epsilon, size_t N, size_t M)
  {
    int numThreads = blockDim.x * gridDim.x;
    int global_id = threadIdx.x + blockIdx.x * blockDim.x;

    for (int id = global_id; id < (N-2)*(M-2); id+=numThreads)
    {
      int i = id/(N-2)+1;
      int j = id%(N-2)+1;
      double vel_x = 1./epsilon[j+i*N]*(hz[j+1+i*N]-hz[j-1+i*N])/2;
      double rhs = vel_x/h;
      double pml_x = 0.5*pow(1.0/2.25, 0.5)*sigma(j*h, 0.01, 20, 200);
      ey[j+i*N] -= dt*(rhs + pml_x*ey[j+i*N])/(1.0 + 0.5*dt*pml_x);
    }
    return;
  }
  
  template<typename T>
  void gpu_solver<T>::iteration(int t, bool to_save)
  {
    dim3 block(256);
    dim3 grid_s((2*(M-50) + block.x-1)/block.x);
    initialize_source<T><<<grid_s, block>>>(t, dt, h, ey, N, M, 30);

    dim3 grid_i( ((N-2)*(M-2) + block.x-1)/block.x );
    iteration_hz<T><<<grid_i, block>>>(h, dt, hz, ex, ey, mu, epsilon, N, M);
    iteration_ex<T><<<grid_i, block>>>(h, dt, hz, ex, ey, mu, epsilon, N, M);
    iteration_ey<T><<<grid_i, block>>>(h, dt, hz, ex, ey, mu, epsilon, N, M);
    cudaDeviceSynchronize();
    if (to_save)
    {
      for (int i = 0; i < M; i++)
      {
        for (int j = 0; j < N; j++)
        {
          output << ey[j + i*(N)] << " ";
        }
        output << "\n";
      }
      output << "\n";
    }
    return;
  }

  template<typename T>
  void gpu_solver<T>::malloc_arrays()
  {
    output.open("data.txt");
    cudaMallocManaged((void **) &ex, N*M*sizeof(T));
    cudaMallocManaged((void **) &ey, N*M*sizeof(T));
    cudaMallocManaged((void **) &hz, N*M*sizeof(T));
    cudaMallocManaged((void **) &mu, N*M*sizeof(T));
    cudaMallocManaged((void **) &epsilon, N*M*sizeof(T));
    dim3 block(256);
    dim3 grid((N*M + block.x-1)/block.x);
    init_const<T><<<grid, block>>>(hz, N*M, 0.0);
    init_const<T><<<grid, block>>>(ey, N*M, 0.0);
    init_const<T><<<grid, block>>>(ex, N*M, 0.0);
    init_const<T><<<grid, block>>>(mu, N*M, 1.0);
    init_circle<T><<<grid, block>>>(epsilon, N, M, 10);
  }

  template<typename T>
  gpu_solver<T>::~gpu_solver()
  {
    output.close();
    cudaFree(ex);
    cudaFree(ey);
    cudaFree(hz);
    cudaFree(mu);
    cudaFree(epsilon);
  }
}
