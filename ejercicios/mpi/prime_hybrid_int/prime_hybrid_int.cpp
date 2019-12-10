#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <iomanip>

int calculate_start(int worker_id, int workers, int finish, int begin)
{
	int range = finish - begin;
	return begin + worker_id * (range / workers) + std::min(worker_id, range % workers);
}

int calculate_finish(int worker_id, int workers, int finish, int begin)
{
	return calculate_start(worker_id + 1, workers, finish, begin);
}

bool is_prime(size_t number)
{
	if ( number < 2 ) return false;
	if ( number == 2 ) return true;
	if ( number % 2 == 0 ) return false;

	for ( size_t i = 3, last = (size_t)(double)sqrt(number); i <= last; i += 2 )
		if ( number % i == 0 )
			return false;

	return true;
}

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

	int global_start = 0;
	int global_finish = 0;
	double start_time = MPI_Wtime();

	if ( argc >= 3 )
	{
		global_start = atoi(argv[1]);
		global_finish = atoi(argv[2]);
	}
	else
	{
		if ( my_rank == 0 )
		{
			std::cin >> global_start >> global_finish;
			for ( int destination = 1; destination < process_count; ++destination )
			{
				MPI_Send(&global_start, 1, MPI_INT, destination, /*tag*/ 0, MPI_COMM_WORLD);
				MPI_Send(&global_finish, 1, MPI_INT, destination, /*tag*/ 0, MPI_COMM_WORLD);
			}
		}
		else
		{
			MPI_Recv(&global_start, 1, MPI_INT, /*source*/ 0, /*tag*/ 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&global_finish, 1, MPI_INT, /*source*/ 0, /*tag*/ 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}

	const int my_start = calculate_start( my_rank, process_count, global_finish, global_start);
	const int my_finish = calculate_finish( my_rank, process_count, global_finish, global_start);
	int prime_count = 0;
	int thread_count = 0;

	#pragma omp parallel default(none)
	{
		thread_count = omp_get_num_threads();
		#pragma omp for schedule( guided ) reduction( +:prime_count )
		for( int i = my_start; i < my_finish; ++i )
			if ( is_prime( i ) )
				++prime_count;
	}
	double elapsed = MPI_Wtime() - start_time;

	if( my_rank == 0 ){
		int process_prime_cnt = 0;
		double process_elapsed = 0;
		int process_thread_cnt = 0;
		for( int sender = 0; sender < process_count; ++sender ){
			MPI_Recv( &process_prime_cnt, 1, MPI_INT, /*source*/ 0, /*tag*/ 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			prime_count += process_prime_cnt;
			MPI_Recv( &process_elapsed, 1, MPI_DOUBLE, /*source*/ 0, /*tag*/ 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			elapsed += process_elapsed;
			MPI_Recv( &process_thread_cnt, 1, MPI_INT, /*source*/ 0, /*tag*/ 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
			thread_count += process_thread_cnt;
		}

		std::cout << prime_count << " primes found in range [" << global_start
			<< "," << global_finish << "[ in " << elapsed << "s with "
			<< process_count << " processes and " << thread_count << " threads";
	} else {
		MPI_Send( &prime_count, 1, MPI_INT, /*dest*/ 0, /*tag*/ 0, MPI_COMM_WORLD );
		MPI_Send( &elapsed, 1, MPI_DOUBLE, /*dest*/ 0, /*tag*/ 0, MPI_COMM_WORLD );
		MPI_Send( &thread_count, 1, MPI_INT, /*dest*/ 0, /*tag*/ 0, MPI_COMM_WORLD );
	}

	MPI_Finalize();
}
