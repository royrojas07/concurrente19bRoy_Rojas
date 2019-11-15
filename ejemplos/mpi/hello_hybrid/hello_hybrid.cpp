#include <iostream>
#include <mpi.h>
#include <omp.h>

int main( int argc, char ** argv ){
	MPI_Init( &argc, &argv );
	
	int my_rank = -1;
	int process_count = -1;
	
	MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &process_count );
	
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int hostname_length = -1;
	MPI_Get_processor_name( hostname, &hostname_length );
	
	std::cout << "Hello from main thread of process " << my_rank << " of " <<
		process_count << " on " << hostname << std::endl;
	
	#pragma omp parallel default(none) shared( my_rank, process_count, hostname, std::cout )
	{
		#pragma omp critical(stdout) 
		{
			std::cout << "\tHello from thread " << my_rank << " of " <<
				process_count << " on " << hostname << std::endl;
		}
	}
	
	MPI_Finalize();
	
	return 0;
}
