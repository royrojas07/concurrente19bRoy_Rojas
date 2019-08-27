#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void * run( void * data ){
	//(void)data;
	size_t thread_num = (size_t)data;
	printf( "HELLO WORLD from secondary thread %zu\n", thread_num );
	return NULL;
}

int main( int argc, char* argv[] ){
	size_t t_count = sysconf(_SC_NPROCESSORS_ONLN);
	if( argc >= 2 )
		t_count = strtoull( argv[1], NULL, 10 );

	pthread_t * threads = malloc( t_count * sizeof(pthread_t) );

	for( size_t index = 0; index < t_count; ++index )
		pthread_create( &threads[index], NULL, run, (void *)index );

	printf( "HELLO WORLD from main thread\n" );

	for( size_t index = 0; index < t_count; ++index )
		pthread_join( threads[index], NULL );

	free(threads);
	return 0;
}
