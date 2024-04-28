#include <fstream>
#include <iostream>
#include <vector>
#include <mpi.h>

#include "cipher.h"


int main(int argc, char ** argv)
{
  //argv[1] - name of the input file
  //argv[2] - name of the output file
  //argv[3] - key
  //argv[4] - num_chunks
  auto ceasar_instance = cipher::ceasar(argc, argv);

  MPI_Barrier(ceasar_instance.comm);
  
  double start = MPI_Wtime();
  ceasar_instance.parallel_ceasar();
  
  MPI_Barrier(ceasar_instance.comm);
  if (ceasar_instance.rank == 0)
  {
    double duration = MPI_Wtime() - start;
    std::cout << "Total duration is: " << duration*1000 << " milliseconds\n";
    double memory = ceasar_instance.len/1024.0/1024.0/duration;
    std::cout << "Bandwidth is: " << memory << "MB/s\n";
  }
  return 0;
}
