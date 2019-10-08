#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

double thread_creation();
void * void_method();

int main( int argc, char * argv[] ){
	size_t trials = 1;
	if( argc == 2 ){
		trials = strtoull( argv[1], NULL, 10 );
	}
	
	double min_thr_creation = 1;
	double thr_creation;
	for( size_t i = 0; i < trials; ++i ){
		thr_creation = thread_creation();
		if( thr_creation < min_thr_creation )
			min_thr_creation = thr_creation;
	}
	
	printf( "Minimum thread creation and destruction time was %.9lfs among %zu trials\n",
		min_thr_creation, trials );
	
	return 0;
}

double thread_creation(){
	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	
	pthread_t thread;
	pthread_create( &thread, NULL, void_method, NULL );
	pthread_join( thread, NULL );
	
	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);
	
	double duration = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);
	
	return duration;
}

void * void_method(){
	return NULL;
}
