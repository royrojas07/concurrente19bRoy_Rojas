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
	int global_min = -1;
	int global_max = -1;
	int global_sum = 0;
	
	MPI_Allreduce( &lucky_number, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
	MPI_Allreduce( &lucky_number, &global_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	MPI_Allreduce( &lucky_number, &global_sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
	double average = (double)global_sum/process_count;
	if( lucky_number == global_min )
		printf( "Process %d: my lucky number (%02d) is the minimum: %02d\n", my_rank, lucky_number, global_min );
	if( lucky_number == global_max )
		printf( "Process %d: my lucky number (%02d) is the maximum: %02d\n", my_rank, lucky_number, global_max );
	if( lucky_number < average )
		printf( "Process %d: my lucky number (%02d) is less than the average: %02f\n", my_rank, lucky_number, average );
	else if( lucky_number > average )
		printf( "Process %d: my lucky number (%02d) is greater than the average: %02f\n", my_rank, lucky_number, average );
	else
		printf( "Process %d: my lucky number (%02d) is the average: %02f\n", my_rank, lucky_number, average );
	
	MPI_Finalize();
	
	return 0;
}
