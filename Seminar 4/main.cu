#include <cmath>
#include <fstream>
#include <iostream>
#include <cuda_runtime.h>
#include <ctime>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cuda_gl_interop.h>
#include "solver.cuh"
#include "gl_viewer.h"

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

template<typename T>
__global__ void init_grad_pos(T * array, T h, size_t N, size_t M)
{
  int numThreads = blockDim.x * gridDim.x;
  int global_id = threadIdx.x + blockIdx.x * blockDim.x;

  for (int id = global_id; id < N*M; id+=numThreads)
  {
    int i = id/N;
    int j = id%N;
    array[3*(j+i*N) + 0] = j*h;
    array[3*(j+i*N) + 1] = i*h;
    array[3*(j+i*N) + 2] = 0.0;
  }
  return;
}

template<typename T, typename S>
__global__ void update_grad_dir(T * array, T h, size_t N, size_t M, S * ey)
{
  int numThreads = blockDim.x * gridDim.x;
  int global_id = threadIdx.x + blockIdx.x * blockDim.x;

  for (int id = global_id; id < (N-2)*(M-2); id+=numThreads)
  {
    int i = id/(N-2)+1;
    int j = id%(N-2)+1;
    float scale = 0.0005;
    array[3*(j+i*N) + 0 + N*M*3] = array[3*(j+i*N) + 0] + scale*(ey[j+1+i*N]-ey[j-1+i*N])/(2.*h);
    array[3*(j+i*N) + 1 + N*M*3] = array[3*(j+i*N) + 1] + scale*(ey[j+(i+1)*N]-ey[j+(i+1)*N])/(2.*h);
    //array[3*(j+i*N) + 2 + N*M*3] = 0.0;
  }
  return;
}



int main(int argc, char ** argv)
{
  std::cout << "inside main\n";

  custom_gl::gl_viewer viewer;//(true);
  int N = 200*10;
  int M = 100*10;
  float h = 0.01/10;
  float dt = 0.004/10;
  int TIME = 1000;

  gpuErrchk(cudaSetDevice(0));
  cudaDeviceProp prop;
  cudaGetDeviceProperties(&prop, 0);
  printf("Device Number: %d\n", 0);
  printf("  Device name: %s\n", prop.name);
  printf("  Memory Clock Rate (KHz): %d\n", prop.memoryClockRate);
  printf("  Memory Bus Width (bits): %d\n", prop.memoryBusWidth);
  printf("  Peak Memory Bandwidth (GB/s): %f\n\n", 
      2.0*prop.memoryClockRate*(prop.memoryBusWidth/8)/1.0e6);


  solver::gpu_solver<float> solver(N, M, h, dt);

  dim3 block(256);
  dim3 grid((N*M + block.x-1)/block.x);

  unsigned int * ebo = new unsigned int [N*M*2];
  for (int i = 0; i < N*M; i++)
  {
    ebo[2*i] = i;
    ebo[2*i+1] = i+N*M;
  }

  float * grads_gl;
  cudaGraphicsResource *vbo_res;  
  viewer.buffer_vbo(N*M*2*3);

  gpuErrchk(cudaGraphicsGLRegisterBuffer(&vbo_res, viewer.VBO, cudaGraphicsMapFlagsNone));
  gpuErrchk(cudaGraphicsMapResources(1, &vbo_res, 0));
  size_t num_bytes;
  gpuErrchk(cudaGraphicsResourceGetMappedPointer((void**)&grads_gl, &num_bytes, vbo_res));

  viewer.buffer_ebo(N*M*2, ebo);
  
  init_grad_pos<<<grid, block>>>(grads_gl, h, N, M);
  init_grad_pos<<<grid, block>>>(&grads_gl[N*M*3], h, N, M);

  viewer.preset();
  
  //while (!glfwWindowShouldClose(viewer.window))
  //{
    cudaDeviceSynchronize();
    clock_t start = clock();
    for (int t = 0; t < TIME; t++)
    {
      dim3 grid_i( ((N-2)*(M-2) + block.x-1)/block.x);
      bool to_save = false;//t % 10 == 0;//false;//
      solver.iteration(t, to_save);
      update_grad_dir<<<grid_i, block>>>(grads_gl, h, N, M, solver.ey);
      cudaDeviceSynchronize();
      viewer.view(N*M*2*3);
    }
    cudaDeviceSynchronize();
    std::cout << "Total computation time: " << (double) (clock() - start)/CLOCKS_PER_SEC << "\n";
  //}
  gpuErrchk(cudaGraphicsUnmapResources(1, &vbo_res, 0));
  delete [] ebo;
  return 0;
}
