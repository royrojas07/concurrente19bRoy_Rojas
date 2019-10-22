#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	size_t t_count;
	size_t next_thread;
	pthread_mutex_t * mutexes;
} shared_data_t;

typedef struct {
	size_t thread_num;
	shared_data_t * shared_data;
} private_data_t;

int create_threads( shared_data_t * shared_data );
void * run( void * data );

int main( int argc, char* argv[] ){
	shared_data_t * shared_data = (shared_data_t*) calloc( 1, sizeof(shared_data_t) );
	if( !shared_data )
		return fprintf( stderr, "error: could not allocate shared memory for %zu threads\n", shared_data->t_count ), 2;

	shared_data->t_count = sysconf(_SC_NPROCESSORS_ONLN);
	if( argc >= 2 )
		shared_data->t_count = strtoull( argv[1], NULL, 10 );
	
	shared_data->next_thread = 0;
	
	pthread_mutex_t * mutex_collection = (pthread_mutex_t*) malloc( shared_data->t_count * sizeof(pthread_mutex_t) );
	shared_data->mutexes = mutex_collection;
	
	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	
	int error = create_threads(shared_data);
	if( error ){
		return error;
	}
	
	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);

	double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

	printf("Hello execution time %.9lfs\n", elapsed_seconds);
	
	for( size_t index = 0; index < shared_data->t_count; ++index )
		pthread_mutex_destroy( &mutex_collection[index] );
	
	free( shared_data );
	return 0;
}

int create_threads( shared_data_t * shared_data ){
	pthread_t * threads = (pthread_t*) malloc( shared_data->t_count * sizeof(pthread_t) );
	if( !threads )
		return fprintf( stderr, "error: could not allocate memory\n" ), 1;
	
	private_data_t * private_data = (private_data_t*) calloc( shared_data->t_count, sizeof(private_data_t) );
	if( !private_data )
		return fprintf( stderr, "error: could not allocate private memory for %zu threads\n", shared_data->t_count ), 3;
	
	for( size_t index = 0; index < shared_data->t_count; ++index ){
		pthread_mutex_init( &shared_data->mutexes[index], NULL );
		if( index != 0 )
			pthread_mutex_lock( &shared_data->mutexes[index] );
		private_data[index].thread_num = index;
		private_data[index].shared_data = shared_data;
		pthread_create( &threads[index], NULL, run, &private_data[index] );
	}
	
	printf( "HELLO WORLD from main thread\n" );

	for( size_t index = 0; index < shared_data->t_count; ++index )
		pthread_join( threads[index], NULL );
	
	free( private_data );
	free( threads );
	return 0;
}

void * run( void * data ){
	private_data_t * private_data = (private_data_t*)data;
	shared_data_t * shared_data = private_data->shared_data;
	pthread_mutex_lock( &shared_data->mutexes[private_data->thread_num] );
	printf( "HELLO WORLD from secondary thread %zu of %zu\n", private_data->thread_num, shared_data->t_count );
	pthread_mutex_unlock( &shared_data->mutexes[private_data->thread_num] );
	if( private_data->thread_num != shared_data->t_count-1 )
		pthread_mutex_unlock( &shared_data->mutexes[private_data->thread_num+1] );
	return NULL;
}
