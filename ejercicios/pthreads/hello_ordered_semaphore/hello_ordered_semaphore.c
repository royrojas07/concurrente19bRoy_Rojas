#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct {
	size_t t_count;
	sem_t * semaphores;
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
	
	sem_t * semaphores_collection = (sem_t*) malloc( shared_data->t_count * sizeof(sem_t) );
	shared_data->semaphores = semaphores_collection;
	
	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	
	int error = create_threads(shared_data);
	if( error ){
		return error;
	}
	
	printf( "HELLO WORLD from main thread\n" );
	
	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);

	double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

	printf("Hello execution time %.9lfs\n", elapsed_seconds);
	
	free( semaphores_collection );
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
	
	sem_init( &shared_data->semaphores[0], 0, 1 );
	for( size_t index = 0; index < shared_data->t_count; ++index ){
		if( index != 0 )
			sem_init( &shared_data->semaphores[index], 0, 0 );
		private_data[index].thread_num = index;
		private_data[index].shared_data = shared_data;
		pthread_create( &threads[index], NULL, run, &private_data[index] );
	}

	for( size_t index = 0; index < shared_data->t_count; ++index )
		pthread_join( threads[index], NULL );
	
	for( size_t index = 0; index < shared_data->t_count; ++index )
		sem_destroy( &shared_data->semaphores[index] );
	
	free( private_data );
	free( threads );
	return 0;
}

void * run( void * data ){
	private_data_t * private_data = (private_data_t*)data;
	shared_data_t * shared_data = private_data->shared_data;
	sem_wait( &shared_data->semaphores[private_data->thread_num] );
	printf( "HELLO WORLD from secondary thread %zu of %zu\n", private_data->thread_num, shared_data->t_count );
	if( private_data->thread_num != shared_data->t_count-1 )
		sem_post( &shared_data->semaphores[private_data->thread_num+1] );
	return NULL;
}
