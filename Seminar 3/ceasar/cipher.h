#include <fstream>
#include <iostream>
#include <vector>
#include <mpi.h>

namespace cipher
{
  class ceasar
  {
    public:
      MPI_Comm comm;
      int size, rank;
      int key, len, num_chunks;
      void parallel_ceasar();
      ceasar(int argc, char ** argv);
      ~ceasar();

    private:
      char * filename;
      MPI_File fh;
      MPI_Status status;
      MPI_Request request;
      int read_file(char const * filename, std::vector<char> &buf);
      void apply_cipher(int key, int len, char * buf);
  };
}
