#include "cipher.h"


namespace cipher
{

  ceasar::ceasar(int argc, char ** argv)
  {
    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    filename = argv[1];
    int rc = MPI_File_open(comm, argv[2], MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
    key = atoi(argv[3]);
    num_chunks = atoi(argv[4]);
  }

  ceasar::~ceasar()
  {
    MPI_File_close(&fh);
    MPI_Finalize();
  }

  void ceasar::parallel_ceasar()
  {
    if (rank == 0)
    {
      std::vector<char> buf;
      len = read_file(filename, buf);

      //1) divide text into pieces
      int chunk_len = len / num_chunks;
      int res = len - chunk_len*num_chunks;

      for(int i = 0; i < num_chunks; i++)
      {
        int n_len = (i < res) ? chunk_len + 1 : chunk_len;
        int offset = (i < res) ? (chunk_len+1)*i : (chunk_len+1)*res + chunk_len*(i-res);

        //2) randomly chooses a process
        int min = 1;
        int max = size-1;
        int dest = min + (rand() % static_cast<int>(max - min + 1));
              
        //3) sends the data
        MPI_Isend(buf.data()+offset, n_len, MPI_CHAR, dest, offset, comm, &request);
      }
      for (int i = 1; i < size; i++)
      {
        MPI_Isend(&i, 0, MPI_INT, i, 777, comm, &request);
      }
    }
    else
    {  
      while(true)
      {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
        MPI_Get_count(&status, MPI_CHAR, &len);
        
        if (len == 0) break;

        char * buf = new char [len];
        MPI_Recv(buf, len, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);

        apply_cipher(key, len, buf);

        MPI_Offset offset = status.MPI_TAG;
        MPI_File_write_at(fh, offset, buf, len, MPI_CHAR, MPI_STATUS_IGNORE);
        delete [] buf;
      }
    }
  }


  int ceasar::read_file(char const * filename, std::vector<char> &buf)
  {
    std::ifstream input;
    input.open(filename, std::ios_base::binary);

    input.seekg(0, input.end);
    int len = input.tellg();
    input.seekg(0, input.beg);

    char * buffer = new char [len];
    input.read(buffer, len);

    buf.reserve(len);
    buf.insert(buf.begin(), buffer, buffer + len);
    delete [] buffer;

    return len;
  }

  void ceasar::apply_cipher(int key, int len, char * buf)
  {
    for (int i =0; i < len; i++)
    {
      if (buf[i] >= 'a' && buf[i] <= 'z')
      {
        buf[i] += key;
        if (buf[i] > 'z' && key > 0)
          buf[i] = buf[i] - 'z' + 'a' - 1;
        else if(buf[i] < 'a' && key < 0)
          buf[i] = buf[i] + 'z' - 'a' + 1;
      }
      if (buf[i] >= 'A' && buf[i] <= 'Z')
      {
        buf[i] += key;
        if (buf[i] > 'Z' && key > 0)
          buf[i] = buf[i] - 'Z' + 'A' - 1;
        else if(buf[i] < 'A' && key < 0)
          buf[i] = buf[i] + 'Z' - 'A' + 1;
      }
    }
    return;
  }
}
