#include <stdio.h>
#include <mpi.h>
#include <random>
#include <iostream>
#include <vector>

int rule(int left, int center, int right){
	if(left == 0 && center == 0 && right == 0){
		return 0;
	} else if(left == 0 && center == 0 && right == 1){
		return 1;
	} else if(left == 0 && center == 1 && right == 0){
		return 0;
	} else if(left == 0 && center == 1 && right == 1){
		return 1;
	} else if(left == 1 && center == 0 && right == 0){
		return 0;
	} else if(left == 1 && center == 0 && right == 1){
		return 1;
	} else if(left == 1 && center == 1 && right == 0){
		return 0;
	} else if(left == 1 && center == 1 && right == 1){
		return 1;
	} else {
		printf("Error in rule!");
		return 0;
	}
}

void apply_rule(int len, std::vector<int>& buf){
	for(int i = 1; i < len-1; i++){
		buf[i] = rule(buf[i-1], buf[i], buf[i+1]);
	}
}

std::vector<int> random_vector(int N){
	std::random_device dev;
    std::mt19937 mt1(dev());
    std::bernoulli_distribution dist(0.5);
	std::vector<int> result(N);
	for(int i = 0; i < N; i++){
		result[i] = dist(mt1);
	}
	return result;
}

void print_vec(const std::vector<int>& vec){
	for(int i = 0; i < vec.size(); i++){
		printf("%d ", vec[i]);
	}
	printf("\n");
}

int main(int argc, char** argv) {
	//argv[1] is file name;
    int rank, size_Of_Cluster;
	int num_chunks = 0;
    int len = 9;
	int N_iterations = 5;
	int periodic = false;
	MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &size_Of_Cluster);
    MPI_Comm_rank(comm, &rank);
    MPI_Status status;
	
	MPI_File fh;
	MPI_File_open(comm, "out.txt", MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);

	num_chunks = size_Of_Cluster;

	//1) divide text into pieces
	int chunk_len = len / num_chunks;
	int res = len - chunk_len*num_chunks;

	if (rank == 0)
	{
		std::vector<int> data = random_vector(len);
		printf("Initial_data = \n");
		print_vec(data);
		for(int i = 0; i < size_Of_Cluster - 1; i++)
		{
			//2) sends the data
			MPI_Send(data.data() + chunk_len*i, chunk_len, MPI_INT, i, 0, comm);
		}
		MPI_Send(data.data() + len - chunk_len - res, chunk_len + res, MPI_INT, size_Of_Cluster - 1, 0, comm);
	}
	std::vector<int> buf;
	if(periodic){
		if(rank == size_Of_Cluster - 1){
			buf.resize(chunk_len + res + 2);
			MPI_Recv(buf.data() + 1, chunk_len + res, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
		} else {
			buf.resize(chunk_len + 2);
			MPI_Recv(buf.data() + 1, chunk_len, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
		}
	} else {
		if(rank == size_Of_Cluster - 1){
			buf.resize(chunk_len + res + 1);
			MPI_Recv(buf.data() + 1, chunk_len + res, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
		} else if (rank == 0){
			buf.resize(chunk_len + 1);
			MPI_Recv(buf.data(), chunk_len, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
		} else {
			buf.resize(chunk_len + 2);
			MPI_Recv(buf.data() + 1, chunk_len, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
		}
	}
	int cnt = 0;
	if(periodic){
		while(cnt < N_iterations){
			MPI_Send(&buf[1], 1, MPI_INT, (size_Of_Cluster + rank - 1) % size_Of_Cluster, 0, comm);
			MPI_Send(&buf[buf.size()-2], 1, MPI_INT, (rank + 1) % size_Of_Cluster, 0, comm);

			MPI_Recv(buf.data(), 1, MPI_INT, (size_Of_Cluster + rank - 1) % size_Of_Cluster, 0, comm, MPI_STATUS_IGNORE);
			MPI_Recv(buf.data() + buf.size() - 1, 1, MPI_INT, (rank + 1) % size_Of_Cluster, 0, comm, MPI_STATUS_IGNORE);

			
			MPI_File_write_at(fh, rank*chunk_len, buf.data()+1, buf.size()-2, MPI_INT, MPI_STATUS_IGNORE);
			if(rank == 0){
				MPI_File_write_at(fh, len, "\n", 1, MPI_CHAR, MPI_STATUS_IGNORE);
			}
			cnt++;
			for(int i = 0; i < size_Of_Cluster; i++)
			{
				if(rank == i){
					printf("Rank = %d\n", i);
					print_vec(buf);	
				}
			}
			MPI_Barrier(comm);
			apply_rule(buf.size(), buf);
		}
	} else {
		while(cnt < N_iterations){
			if(rank == 0){
				MPI_Send(&buf[buf.size()-2], 1, MPI_INT, 1, 0, comm);
				MPI_Recv(buf.data() + buf.size() - 1, 1, MPI_INT, 1, 0, comm, MPI_STATUS_IGNORE);
			} else if (rank == size_Of_Cluster - 1){
				MPI_Send(&buf[1], 1, MPI_INT, size_Of_Cluster - 2, 0, comm);
                MPI_Recv(buf.data(), 1, MPI_INT, size_Of_Cluster - 2, 0, comm, MPI_STATUS_IGNORE);
			} else {
				MPI_Send(&buf[1], 1, MPI_INT, (size_Of_Cluster + rank - 1) % size_Of_Cluster, 0, comm);
				MPI_Send(&buf[buf.size()-2], 1, MPI_INT, (rank + 1) % size_Of_Cluster, 0, comm);

				MPI_Recv(buf.data(), 1, MPI_INT, (size_Of_Cluster + rank - 1) % size_Of_Cluster, 0, comm, MPI_STATUS_IGNORE);
				MPI_Recv(buf.data() + buf.size() - 1, 1, MPI_INT, (rank + 1) % size_Of_Cluster, 0, comm, MPI_STATUS_IGNORE);
			}
			MPI_File_write_at(fh, rank*chunk_len, buf.data()+1, buf.size()-2, MPI_INT, MPI_STATUS_IGNORE);
			if(rank == 0){
				MPI_File_write_at(fh, len, "\n", 1, MPI_CHAR, MPI_STATUS_IGNORE);
			}
			cnt++;
			for(int i = 0; i < size_Of_Cluster; i++)
            {
                if(rank == i){
                    printf("Rank = %d\n", i);
                    print_vec(buf);
                }
            }
			MPI_Barrier(comm);
			apply_rule(buf.size(), buf);
		}
	}

	MPI_File_close(&fh);

	MPI_Finalize();
	return 0;
}
