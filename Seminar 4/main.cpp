#include <cmath>
#include <fstream>
#include <iostream>
#include <omp.h>

void initialize_source(int t, double dt, double h, double * field, int N, int M, int k)
{
  #pragma omp parallel for collapse(2)
  for (int i = 25; i < M-25; i++)
    for (int j = k; j < k+2; j++)
      field[j+i*N] = sin(10*3.1416*(t*dt-j*h));
}

void init_ones(double *array, int N)
{
  #pragma omp parallel for
  for (int i = 0; i < N; i++) array[i] = 1.0;
}

void init_circle(double *array, int N, int M, int R)
{
  #pragma omp parallel for collapse(2)
  for (int i = 0; i < M; i++) 
    for (int j = 0; j < N; j++)
      if ((i-M/2)*(i-M/2) + (j-N/2)*(j-N/2) < R*R)
        array[j+i*N] = 5.25;
}

double sigma(double const &r, double const &dr, int const &PML, int const & size)
{
  if (r>(dr*(size-PML))) return pow((r-(size-PML)*dr)/(PML*dr),2)*3.0*log(10.0)*13.0/(PML*dr);
	else if (r < dr*(PML)) return pow((PML*dr-r)/(PML*dr),2)*3.0*log(10.0)*13.0/(PML*dr);
	else return 0.0;
}

int main(int argc, char ** argv)
{
  int N = 200*10;
  int M = 100*10;
  float h = 0.01/10;
  float dt = 0.004/10;
  int TIME = 1000;

  double * ex = new double [N*M];
  double * ey = new double [N*M];
  double * hz = new double [N*M];
  double * mu = new double [N*M];
  init_ones(mu, N*M);
  double * epsilon = new double [N*M];
  init_ones(epsilon, N*M);
  init_circle(epsilon, N, M, 10);

  std::ofstream output;
  output.open("data.txt");


  double start = omp_get_wtime();
  for (int t = 0; t < TIME; t++)
  {
    initialize_source(t, dt, h, ey, N, M, 30);
    #pragma omp parallel for collapse(2)
    for (int i = 1; i < M-1; i++)
    {
      for (int j = 1; j < N-1; j++)
      {
        double vel_x = 1./mu[j+i*N]*(ey[j+1+i*N]-ey[j-1+i*N])/2;
        double vel_y = -1./mu[j+i*N]*(ex[j+(i+1)*N]-ex[j+(i-1)*N])/2;
        double rhs = vel_x/h+vel_y/h;
        double pml_y = 0.5*pow(1.0/2.25, 0.5)*sigma(i*h, 0.01, 10, 100);
        double pml_x = 0.5*pow(1.0/2.25, 0.5)*sigma(j*h, 0.01, 20, 200);
        hz[j+i*N] -= dt*(rhs + pml_y*hz[j+i*N] + pml_x*hz[j+i*N])/(1.0+0.5*dt*(pml_x + pml_y));
      }
    }
    #pragma omp parallel for collapse(2)
    for (int i = 1; i < M-1; i++)
    {
      for (int j = 1; j < N-1; j++)
      {
        double vel_y = -1./epsilon[j+i*N]*(hz[j+(i+1)*N]-hz[j+(i-1)*N])/2;
        double rhs = vel_y/h;
        double pml_y = 0.5*pow(1.0/2.25, 0.5)*sigma(i*h, 0.01, 10, 100);
        ex[j+i*N] -= dt*(rhs + pml_y*ex[j+i*N])/(1.0 + 0.5*dt*pml_y);
      }
    }
    #pragma omp parallel for collapse(2)
    for (int i = 1; i < M-1; i++)
    {
      for (int j = 1; j < N-1; j++)
      {
        double vel_x = 1./epsilon[j+i*N]*(hz[j+1+i*N]-hz[j-1+i*N])/2;
        double rhs = vel_x/h;
        double pml_x = 0.5*pow(1.0/2.25, 0.5)*sigma(j*h, 0.01, 20, 200);
        ey[j+i*N] -= dt*(rhs + pml_x*ey[j+i*N])/(1.0 + 0.5*dt*pml_x);
      }
    }

    if (false)//t % 10 == 0)
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
  }
  std::cout << "Total computation time: " << omp_get_wtime() - start << "\n";
  delete [] ex;
  delete [] ey;
  delete [] hz;
  output.close();
  return 0;
}
