#include <stdio.h>
#include <pthread.h>

void * run( void * data ){
	//(void)data;
	size_t thread_num = (size_t)data;
	printf( "HELLO WORLD from secondary thread %zu\n", thread_num );
	return NULL;
}

int main(){
	pthread_t thread;
	pthread_create( &thread, NULL, run, (void *)31 );
	printf( "HELLO WORLD from main thread\n" );
	pthread_join( thread, NULL );
	return 0;
}
