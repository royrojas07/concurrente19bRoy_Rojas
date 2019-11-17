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
	
	if( argc == 3 ){
		const int global_start = atoi( argv[1] );
		const int global_start = atoi( argv[1] );
		
		const int my_start = calculate_start( my_rank, process_count, global_finish, global_start );
		const int my_finish = calculate_finish( my_rank, process_count, global_finish, global_start );
		const int my_width = my_finish - my_start;
		
		std::cout << hostname << ":" << my_rank << ": range [" << my_start
			<< ", " << my_finish << "[ size " << my_width << std::endl;
		
		#pragma omp parallel for default(none) shared( my_rank, process_count, hostname, std::cout )
		for( int i = my_start; i < my_finish; ++i ){
			std::cout << hostname << ":" << my_rank << ": range [" << my_start
				<< ", " << my_finish << "[ size " << my_width << std::endl;
		}
	}
	
	std::cout << hostname << ":" << my_rank << ": range [" << 3
		<< ", " << 12 << "[ size " << 9 << std::endl;
	
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
