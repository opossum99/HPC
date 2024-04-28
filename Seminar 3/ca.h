#include <fstream>
#include <iostream>
#include <vector>
#include <mpi.h>

namespace ca
{
  class ca1d
  {
    public:
      int size, rank;
      char * rule;
      int len;//size of subarray
      void parallel_ca();
      ca1d(int argc, char ** argv);
      ~ca1d();

    private:
      char * filename;
      MPI_File fh;
      MPI_Comm comm;
      MPI_Status status;
      MPI_Request request;
      void apply_iteration(int i);
  };
}
