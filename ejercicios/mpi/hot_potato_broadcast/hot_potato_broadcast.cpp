#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <iomanip>

typedef struct{
	int potato;
	int no_losing_processes_count;
	int root_process;
} data_t;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int my_rank = -1;
	int process_count = -1;

	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);

	data_t * data = ( data_t * ) calloc( 1, sizeof( data_t ) );

	int initial_potato = 0;
	int initial_process = 0;
	int next = my_rank+1 == process_count ? 0 : my_rank+1;
	data->no_losing_processes_count = process_count;
	bool lose = false;
	bool game_over = false;

	if( argc == 3 ){
		initial_potato = atoi( argv[1] );
		initial_process = atoi( argv[2] );
	}
	else if( my_rank == 0 )
		return std::cout << "usage: ./hot_potato_winner potato_value initial_process" << std::endl, 1;

	data->potato = initial_potato;
	data->root_process = initial_process;
	if( data->potato > 0 && initial_process >= 0 && initial_process < process_count ){
		while( !game_over ){
			if( my_rank == data->root_process ){
				if( !lose ){
					if( data->no_losing_processes_count == 1 ){
						game_over = true;
						data->potato = 0;
						std::cout << "Process " << my_rank << " won!" << std::endl;
					} else {
						if( ( data->potato % 2 ) == 0 )
							data->potato /= 2;
						else
							data->potato = data->potato*3 + 1;
						if( data->potato == 1 ){
							lose = true;
							data->potato = initial_potato;
							--data->no_losing_processes_count;
						}
					}
				}
				else if( data->potato == 0 )
					game_over = true;

				data->root_process = next;
				MPI_Bcast( &data, 3, MPI::INT, my_rank, MPI_COMM_WORLD );
			}
			else
				MPI_Bcast( &data, 3, MPI::INT, data->root_process, MPI_COMM_WORLD );
		}
	}

	free( data );

	MPI_Finalize();
}
