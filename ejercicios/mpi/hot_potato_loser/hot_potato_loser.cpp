#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <iomanip>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int my_rank = -1;
	int process_count = -1;

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);

	int potato = 0;

	if( argc == 2 )
		potato = atoi( argv[1] );
	else if( my_rank == 0 )
		std::cout << "usage: ./hot_potato_loser potato_value" << std::endl;

	int iteration = 0;
	while( potato > 0 && process_count != 1 ){
		if( my_rank != 0 )
			MPI_Recv( &potato, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		else if( iteration != 0 )
			MPI_Recv( &potato, 1, MPI_INT, process_count-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		--potato;
		if( potato == 0 )
			std::cout << "Potato exploded in process " << my_rank << std::endl;

		if( ( potato + process_count ) != 1 ){
			if( my_rank == process_count-1 )
				MPI_Send( &potato, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );
			else
				MPI_Send( &potato, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD );
		}
		++iteration;
	}

	MPI_Finalize();
}
