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

	char hostname[MPI_MAX_PROCESSOR_NAME];
	int hostname_length = -1;
	MPI_Get_processor_name(hostname, &hostname_length);

	int initial_potato = 0;
	int potato = 0;
	int no_losing_processes_count = process_count;
	bool lose = false;
	bool just_lost = false;
	bool game_over = false;
	bool other_lose = false;

	if( argc == 2 )
		initial_potato = atoi( argv[1] );
	else if( my_rank == 0 )
		std::cout << "usage: ./hot_potato_winner potato_value" << std::endl;

	potato = initial_potato;
	int iteration = 0;
	while( !game_over && potato > 0 ){
		if( iteration != 0 ){
			for( int i = my_rank+1; i < process_count + my_rank; ++i ){
				MPI_Bcast( &other_lose, 1, MPI_BOOL, (i % process_count), MPI_COMM_WORLD );
				if( other_lose )
					--no_losing_processes_count;
			}
			if( my_rank != 0 )
				MPI_Recv( &potato, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			else
				MPI_Recv( &potato, 1, MPI_INT, process_count-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		} else if( my_rank != 0 ){
			for( int i = 0; i < my_rank; ++i ){
				MPI_Bcast( &other_lose, 1, MPI_BOOL, i, MPI_COMM_WORLD );
				if( other_lose )
					--no_losing_processes_count;
			}
			MPI_Recv( &potato, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		}

		if( !lose ){
			if( potato == initial_potato && no_losing_processes_count == 1 ){
				game_over = true;
				potato = 0;
				std::cout << "Process " << my_rank << " won!" << std::endl;
				if( my_rank == process_count-1 )
					MPI_Send( &potato, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );
				else
					MPI_Send( &potato, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD );
			} else {
				--potato;
				if( potato == 0 ){
					lose = true;
					just_lost = true;
					potato = initial_potato;
				}
			}
		} else {
			if( potato <= 0 ){
				game_over = true;
				--potato;
			}
		}

		MPI_Bcast( &just_lost, 1, MPI_BOOL, my_rank, MPI_COMM_WORLD );

		if( ( potato + process_count ) != 1 ){
			if( my_rank == process_count-1 )
				MPI_Send( &potato, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );
			else
				MPI_Send( &potato, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD );
		}

		++iteration;
		just_lost = false;
	}

	for( int i = my_rank+1; i < process_count + my_rank + potato; ++i ){
		MPI_Bcast( &other_lose, 1, MPI_BOOL, (i % process_count), MPI_COMM_WORLD );
	}

	MPI_Finalize();
}
