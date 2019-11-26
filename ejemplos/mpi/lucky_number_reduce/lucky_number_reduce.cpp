#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <iomanip>
#include <ctime>

int main( int argc, char ** argv ){
	MPI_Init( &argc, &argv );
	
	int my_rank = -1;
	int process_count = -1;
	
	MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
	MPI_Comm_size( MPI_COMM_WORLD, &process_count );
	
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int hostname_length = -1;
	MPI_Get_processor_name( hostname, &hostname_length );
	
	srand( my_rank + time(nullptr) + clock() );
	int lucky_number = rand() % 100;
	
	//std::cout << "Process " << my_rank << ": my lucky number is "
		//<< std::setw(2) << std::setfill('0') << lucky_number << std::endl;
	printf( "Process %d: my lucky number is %02d\n", my_rank, lucky_number );
	
	int global_min = -1;
	int global_max = -1;
	int global_sum = 0;
	
	MPI_Reduce( &lucky_number, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce( &lucky_number, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce( &lucky_number, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	//std::cout << "Process " << my_rank << ": all minimum: "
		//<< std::setw(2) << std::setfill('0') << global_min << std::endl;
	if( my_rank == 0 ){
		double average = (double)global_sum/process_count;
		printf( "Process %d: all minimum: %02d\n", my_rank, global_min );
		printf( "Process %d: all maximum: %02d\n", my_rank, global_max );
		printf( "Process %d: all average: %f\n", my_rank, average );
	}
	
	MPI_Finalize();
	
	return 0;
}
