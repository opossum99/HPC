#include <stdio.h>
#include <mpi.h>
#include <fstream>
#include <random>
#include <iostream>

int main(int argc, char** argv) {
	// argv[1] is number of iteration.	
	// argv[2] is size_of_message.
	
	
    int process_Rank, size_Of_Cluster;
    int N = std::atoi(argv[1]); //Number of iterations.

	int size_of_message = std::atoi(argv[2]); // It should be greater than 1.
	
	std::cout << "Size of message = " << size_of_message << std::endl;

    //The first element shows the number of iterations. 
	//The second element shows the process Rank.
    std::vector<int> message(size_of_message);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    MPI_Status status;

	std::random_device dev;
    std::mt19937 mt1(dev());
    std::uniform_int_distribution<int> dist(0, size_Of_Cluster - 1);

	for(int i = 1; i < size_of_message; i++){
		message[i] = dist(mt1);
	}

	printf("begin\n\n");

	message[0] = 0;
	message[1] = 0;

	double start = MPI_Wtime();
    while(message[0] < N){
		int next_proc = dist(mt1);
		if(next_proc != process_Rank){
			if(message[1] == process_Rank){
				message[1] = next_proc;
// 				printf("\nSend from %d to %d!-------------------------------------------------\n", process_Rank, next_proc);
// 				for(int i = 0; i < message.size(); i++){
// 					printf("%d  ", message[i]);
// 				}
// 				printf("\n-----------------------------------------------------------------------------------\n");
				MPI_Ssend(message.data(), size_of_message, MPI_INT, next_proc, 0, MPI_COMM_WORLD);
			} else {
				MPI_Recv(message.data(), size_of_message, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
// 				printf("\nReceive %d!-------------------------------------------------\n", process_Rank);
// 				for(int i = 0; i < message.size(); i++){
// 					printf("%d  ", message[i]);
// 				}
// 				printf("\n-----------------------------------------------------------------\n");
				message[0]++;
			}
		}
	}
    
	if(message[1] == process_Rank){
		std::ofstream MyFile;
		MyFile.open("table.csv", std::ios::app);
		double size_in_bytes = size_of_message*sizeof(int); 
		double duration = MPI_Wtime() - start;
		double memory = size_in_bytes/(1024*1024)/duration;
		MyFile << size_in_bytes << ", " << N << ", " << duration << ", " << duration/N << ", " << memory << std::endl;
    	std::cout << "Bandwidth is: " << memory << "MB/s\n";
//      printf("Send rest===============================================================================================================================\n");
        for(int i = 0; i < size_Of_Cluster; i++){
            if(i != message[1]){
                MPI_Send(message.data(), message.size(), MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        }
    }
    
    MPI_Finalize();
    return 0;
}
