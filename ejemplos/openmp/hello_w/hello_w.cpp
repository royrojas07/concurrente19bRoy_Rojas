#include <iostream>
#include <cstdlib>
#include <omp.h>

int main( int argc, char * argv[] ){
	int thread_count = omp_get_max_threads();
	if( argc >= 2 )
		thread_count = atoi(argv[1]);
	#pragma omp parallel num_threads( thread_count )
	{
		#pragma omp critical(cout)
		std::cout << "Hello world from secondary thread " << omp_get_thread_num()
			<< " of " << omp_get_num_threads() << std::endl;
	}
	
	return 0;
}
