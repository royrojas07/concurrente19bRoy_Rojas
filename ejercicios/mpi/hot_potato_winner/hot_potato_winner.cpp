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

	int initial_potato = 0;
	int potato = 0;
	int no_losing_processes_count = process_count;
	int prev = my_rank-1 < 0 ? process_count-1 : my_rank-1;
	int next = my_rank+1 == process_count ? 0 : my_rank+1;
	bool lose = false;
	bool game_over = false;

	if( argc == 2 )
		initial_potato = atoi( argv[1] );
	else if( my_rank == 0 )
		std::cout << "usage: ./hot_potato_winner potato_value" << std::endl;

	potato = initial_potato;
	int iteration = 0;
	if( potato > 0 ){
		while( !game_over ){
			if( iteration != 0 || my_rank != 0 ){
				MPI_Recv( &potato, 1, MPI::INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
				MPI_Recv( &no_losing_processes_count, 1, MPI::INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			}

			if( !lose ){
				if( no_losing_processes_count == 1 ){
					game_over = true;
					potato = 0;
					std::cout << "Process " << my_rank << " won!" << std::endl;
				} else {
					--potato;
					if( potato == 0 ){
						lose = true;
						potato = initial_potato;
						--no_losing_processes_count;
					}
				}
			} else if( potato <= 0 ){
				game_over = true;
				--potato;
			}

			if( ( potato + process_count ) != 1 ){
				MPI_Send( &potato, 1, MPI::INT, next, 0, MPI_COMM_WORLD );
				MPI_Send( &no_losing_processes_count, 1, MPI::INT, next, 0, MPI_COMM_WORLD );
			}

			++iteration;
		}
	}

	MPI_Finalize();
}
