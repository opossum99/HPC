#include <stdio.h>
#include <mpi.h>
#include <random>
#include <iostream>

int main(int argc, char** argv) {
    int process_Rank, size_Of_Cluster;
    int N = 6;
    std::vector<int> procs;
    procs.emplace_back(0);

	int number_of_proc;

  

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
    MPI_Status status;

    std::random_device dev;
    std::mt19937 mt1(dev());
    std::uniform_int_distribution<int> dist(0, size_Of_Cluster - 1);

    while(procs.size() < N){
		int next_proc = dist(mt1);
		if(next_proc != process_Rank){
	    	if(procs.back() == process_Rank){
	        	procs.emplace_back(next_proc);
				printf("\n\nSend from %d proc!-----------------------------------------------------\n", process_Rank);
	        	for(int i = 0; i < procs.size(); i++){
		    		printf("%d  ", procs[i]);
	        	}
				printf("\n-----------------------------------------------------------------------------------\n");
	        	MPI_Ssend(procs.data(), procs.size(), MPI_INT, next_proc, 0, MPI_COMM_WORLD);	        
	    	} else {
				MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
				MPI_Get_count(&status, MPI_INT, &number_of_proc);
				printf("Size of array = %d\n", number_of_proc);
				procs.resize(number_of_proc);
	        	MPI_Recv(procs.data(), procs.size(), MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	        	printf("\nRecieve %d!-------------------------------------------------\n", process_Rank);
	        	for(int i = 0; i < procs.size(); i++){
		    		printf("%d  ", procs[i]);
	        	}
				printf("\n-----------------------------------------------------------------\n");
	    	}
		}
    }

    if(procs.back() == process_Rank){
		printf("Send rest===============================================================================================================================\n");
        for(int i = 0; i < size_Of_Cluster; i++){
			if(i != procs[procs.size()-2] && i!= procs.back()){
	    		MPI_Send(procs.data(), procs.size(), MPI_INT, i, 0, MPI_COMM_WORLD);
			}
		}
    }

    
    MPI_Finalize();
    return 0;
}
