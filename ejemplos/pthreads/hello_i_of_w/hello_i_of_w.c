#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
	size_t t_count;
} shared_data_t;

typedef struct {
	size_t thread_num;
	shared_data_t * shared_data;
} private_data_t;

void * run( void * data ){
	private_data_t * private_data = (private_data_t*)data;
	shared_data_t * shared_data = private_data->shared_data;
	printf( "HELLO WORLD from secondary thread %zu of %zu\n", private_data->thread_num, shared_data->t_count );
	return NULL;
}

int main( int argc, char* argv[] ){
	size_t t_count = sysconf(_SC_NPROCESSORS_ONLN);
	if( argc >= 2 )
		t_count = strtoull( argv[1], NULL, 10 );

	pthread_t * threads = (pthread_t*) malloc( t_count * sizeof(pthread_t) );
	
	if( !threads )
		return fprintf( stderr, "error: could not allocate memory for %zu threads\n", t_count ), 1;
	
	shared_data_t * shared_data = (shared_data_t*) calloc( 1, sizeof(shared_data_t) );
	if( !shared_data )
		return fprintf( stderr, "error: could not allocate shared memory for %zu threads\n", t_count ), 2;
	
	shared_data->t_count = t_count;
	
	private_data_t * private_data = (private_data_t*) calloc( t_count, sizeof(private_data_t) );
	if( !private_data )
		return fprintf( stderr, "error: could not allocate private memory for %zu threads\n", t_count ), 3;
	
	for( size_t index = 0; index < t_count; ++index ){
		private_data[index].thread_num = index;
		private_data[index].shared_data = shared_data;
		pthread_create( &threads[index], NULL, run, &private_data[index] );
	}
	
	printf( "HELLO WORLD from main thread\n" );

	for( size_t index = 0; index < t_count; ++index )
		pthread_join( threads[index], NULL );
	
	free( private_data );
	free( shared_data );
	free( threads );
	return 0;
}
