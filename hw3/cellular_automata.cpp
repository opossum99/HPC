#include <stdio.h>
#include <mpi.h>
#include <random>
#include <iostream>
#include <vector>

int rule(int left, int center, int right){// Analogue to the game "Life"
	if(left == 0 && center == 0 && right == 0){
		return 0;
	} else if(left == 0 && center == 0 && right == 1){
		return 0;
	} else if(left == 0 && center == 1 && right == 0){
		return 0;
	} else if(left == 0 && center == 1 && right == 1){
		return 1;
	} else if(left == 1 && center == 0 && right == 0){
		return 0;
	} else if(left == 1 && center == 0 && right == 1){
		return 1;
	} else if(left == 1 && center == 1 && right == 0){
		return 1;
	} else if(left == 1 && center == 1 && right == 1){
		return 0;
	} else {
		printf("Error in rule!");
		return 0;
	}
}

void apply_rule(int len, std::vector<int>& buf){
	int tmp1 = rule(buf[0], buf[1], buf[2]);
	int tmp2;
	for(int i = 2; i < len-1; i++){
		tmp2 = rule(buf[i-1], buf[i], buf[i+1]);
		buf[i-1] = tmp1;
		tmp1 = tmp2;
	}
	buf[len - 2] = tmp1;
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

std::vector<int> particular_vector(int N){
	std::vector<int> result(N, 1);
	result[0] = 0;
	result[N - 1] = 0;
	return result;
}

void print_vec(const std::vector<int>& vec){
	for(int i = 0; i < vec.size(); i++){
		printf("%d ", vec[i]);
	}
	printf("\n");
}

void print_vec_file(const std::vector<int>& vec, FILE *fptr){
	for(int i = 0; i < vec.size(); i++){
		fprintf(fptr, "%d", vec[i]);
	}
	fprintf(fptr, "\n");
}

int main(int argc, char** argv) {
    int rank, size_Of_Cluster;
	int num_chunks = 0;
    int len = 100000; // sample size
	int N_iterations = 4000; // number of iterations
	int periodic = true; // periodic or constant boundary conditions
	int writable = false; // necessary to write results in file
	MPI_Comm comm = MPI_COMM_WORLD;
	FILE *file_speedup; // file for graph of speedup
	FILE *f; // data logger

    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &size_Of_Cluster);
    MPI_Comm_rank(comm, &rank);
    MPI_Status status;

	num_chunks = size_Of_Cluster;

	//1) divide text into pieces
	int chunk_len = len / num_chunks;
	int res = len - chunk_len*num_chunks;
	std::vector<int> buf;
	std::vector<int> result;

	double time_one_process = 0;
	if (rank == 0)
	{
		std::cout << "Income " << size_Of_Cluster << std::endl;
		std::vector<int> data = random_vector(len);
		result = data;
// 		printf("Initial_data = \n");
// 		print_vec(data);
		if(periodic){
			buf.resize(1);
			buf.insert(buf.end(), data.begin(), data.begin()+chunk_len+1);
		} else {
			buf.insert(buf.end(), data.begin(), data.begin()+chunk_len+1);
		}
		for(int i = 1; i < size_Of_Cluster - 1; i++)
		{
			//2) sends the data
			MPI_Send(data.data() + chunk_len*i, chunk_len, MPI_INT, i, 0, comm);
// 			std::cout << "Send_0_data" << std::endl;
		}
		MPI_Send(data.data() + len - chunk_len - res, chunk_len + res, MPI_INT, size_Of_Cluster - 1, 0, comm);
// 		std::cout << "Send_data" << std::endl;

		double start_time = MPI_Wtime();
		if(periodic){
			int tmp;
			for(int i = 0; i < N_iterations; i++){
				apply_rule(data.size(), data);
				tmp = rule(data[data.size()-1], data[0], data[1]);
				data[data.size()-1] = rule(data[data.size()-2], data[data.size()-1], data[0]);
				data[0] = tmp;
			}
		} else {
			for(int i = 0; i < N_iterations; i++){
				apply_rule(data.size(), data);
			}
		}
		time_one_process = MPI_Wtime() - start_time;
		std::cout << "Time of one process " << time_one_process << std::endl;
	}
	if(rank > 0 && periodic){
		if(rank == size_Of_Cluster - 1){
			buf.resize(chunk_len + res + 2);
			MPI_Recv(buf.data() + 1, chunk_len + res, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
		} else {
			buf.resize(chunk_len + 2);
			MPI_Recv(buf.data() + 1, chunk_len, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
		}
	} else if (rank > 0){
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



	double start = MPI_Wtime();
	f = fopen("out.txt", "w");
	int cnt = 0;
	if(periodic){
		while(cnt < N_iterations){
			MPI_Send(&buf[1], 1, MPI_INT, (size_Of_Cluster + rank - 1) % size_Of_Cluster, 0, comm);
			MPI_Send(&buf[buf.size()-2], 1, MPI_INT, (rank + 1) % size_Of_Cluster, 0, comm);

			MPI_Recv(buf.data(), 1, MPI_INT, (size_Of_Cluster + rank - 1) % size_Of_Cluster, 0, comm, MPI_STATUS_IGNORE);
			MPI_Recv(buf.data() + buf.size() - 1, 1, MPI_INT, (rank + 1) % size_Of_Cluster, 0, comm, MPI_STATUS_IGNORE);
			
			cnt++;
// 			for(int i = 0; i < size_Of_Cluster; i++)
// 			{
// 				if(rank == i){
// 					printf("Rank = %d\n", i);
// 					print_vec(buf);	
// 				}
// 			}
			if(writable && rank == 0){
				print_vec_file(result, f);
// 				printf("Result %d:\n", cnt);
// 				print_vec(result);
			}
			MPI_Barrier(comm);
			apply_rule(buf.size(), buf);
			if(writable){
				MPI_Send(&buf[1], buf.size() - 2, MPI_INT, 0, 0, comm);
				if(rank == 0){
					for(int i = 0; i < size_Of_Cluster; i++){
						int buf_size; // for last process
						MPI_Probe(MPI_ANY_SOURCE, 0, comm, &status);
						MPI_Get_count(&status, MPI_INT, &buf_size);
						MPI_Recv(result.data() + status.MPI_SOURCE*(buf.size() - 2), buf_size, MPI_INT, MPI_ANY_SOURCE, 0, comm, MPI_STATUS_IGNORE);
					}
				}
				MPI_Barrier(comm);
			}
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

			cnt++;
// 			for(int i = 0; i < size_Of_Cluster; i++)
//             {
//                 if(rank == i){
//                     printf("Rank = %d\n", i);
//                     print_vec(buf);
//                 }
//             }
			if(writable && rank == 0){
				print_vec_file(result, f);
// 				printf("Result %d:\n", cnt);
// 				print_vec(result);
			}
			MPI_Barrier(comm);
			apply_rule(buf.size(), buf);
			if(writable){
				if(rank == 0){
					MPI_Send(&buf[0], buf.size() - 1, MPI_INT, 0, 0, comm);
				} else if (rank == size_Of_Cluster - 1){
					MPI_Send(&buf[1], buf.size() - 1, MPI_INT, 0, 0, comm);
				} else {
					MPI_Send(&buf[1], buf.size() - 2, MPI_INT, 0, 0, comm);
				}
				if(rank == 0){
					for(int i = 0; i < size_Of_Cluster; i++){
						int buf_size; // for last process
						MPI_Probe(MPI_ANY_SOURCE, 0, comm, &status);
						MPI_Get_count(&status, MPI_INT, &buf_size);
						MPI_Recv(result.data() + status.MPI_SOURCE*(buf.size() - 1), buf_size, MPI_INT, MPI_ANY_SOURCE, 0, comm, MPI_STATUS_IGNORE);
					}
				}
				MPI_Barrier(comm);
			}
		}
	}
	fclose(f);

	if(rank == 0){
		file_speedup = fopen("speedup.txt", "a");
		fprintf(file_speedup, "%d  %f\n", size_Of_Cluster, time_one_process/(MPI_Wtime() - start));
		fclose(file_speedup);
	}


	MPI_Finalize();
	return 0;
}
