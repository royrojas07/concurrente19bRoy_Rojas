#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct {
	pthread_mutex_t stdout_mutex;
	size_t buffer_size;
	double * buffer;
	size_t rounds;
	useconds_t min_producer_delay;
	useconds_t max_producer_delay;
	useconds_t min_consumer_delay;
	useconds_t max_consumer_delay;
	sem_t producer_semaphore;
	sem_t consumer_semaphore;
} shared_data_t;

int analize_arguments( int argc, char* argv[], shared_data_t * shared_data );
int create_threads( shared_data_t * shared_data );
void * produce( void * data );
void * consume( void * data );
void random_sleep( useconds_t min_milliseconds, useconds_t max_milliseconds );

int main( int argc, char* argv[] ){
	srand( time(NULL) );
	shared_data_t * shared_data = (shared_data_t*) calloc( 1, sizeof(shared_data_t) );

	int error = analize_arguments( argc, argv, shared_data );
	if( !error ){
		shared_data->buffer = (double*) calloc( shared_data->buffer_size, sizeof(double) );
		if( shared_data->buffer ){
			sem_init( &shared_data->producer_semaphore, 0, shared_data->buffer_size );
			sem_init( &shared_data->consumer_semaphore, 0, 0 );
			pthread_mutex_init( &shared_data->stdout_mutex, NULL );
			
			struct timespec start_time;
			clock_gettime(CLOCK_MONOTONIC, &start_time);
			
			error = create_threads(shared_data);
			if( !error ){
				struct timespec finish_time;
				clock_gettime(CLOCK_MONOTONIC, &finish_time);

				double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
					+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

				printf("Simulation time %.9lfs\n", elapsed_seconds);
			}
			pthread_mutex_destroy( &shared_data->stdout_mutex );
			sem_destroy( &shared_data->consumer_semaphore );
			sem_destroy( &shared_data->producer_semaphore );
		} else {
			fprintf( stderr, "error: could not allocate memory for %zu products\n", shared_data->buffer_size );
			error = 2;
		}
	}
	
	free( shared_data );
	return 0;
}

int analize_arguments( int argc, char* argv[], shared_data_t * shared_data ){
	if( argc == 7 ){
		shared_data->buffer_size = strtoull( argv[1], NULL, 10 );
		if( shared_data->buffer_size == 0 )
			return 2;
		if( sscanf( argv[2], "%zu", &shared_data->rounds ) != 1 || shared_data->rounds == 0 )
			return (void)fprintf( stderr, "invalid rounds: %s\n", argv[2] ), 2;
		if( sscanf( argv[3], "%u", &shared_data->min_producer_delay ) != 1 )
			return (void)fprintf( stderr, "invalid min_producer_delay: %s\n", argv[3] ), 3;
		if( sscanf( argv[4], "%u", &shared_data->max_producer_delay ) != 1 )
			return (void)fprintf( stderr, "invalid max_producer_delay: %s\n", argv[4] ), 4;
		if( sscanf( argv[5], "%u", &shared_data->min_consumer_delay ) != 1 )
			return (void)fprintf( stderr, "invalid min_consumer_delay: %s\n", argv[5] ), 5;
		if( sscanf( argv[6], "%u", &shared_data->max_consumer_delay ) != 1 )
			return (void)fprintf( stderr, "invalid max_consumer_delay: %s\n", argv[6] ), 6;
	} else {
		fprintf( stderr, "producer_consumer buffer_size rounds"
			" min_producer_delay max_producer_delay"
			" min_consumer_delay max_consumer_delay\n");
		return 1;
	}
	return 0;
}

int create_threads( shared_data_t * shared_data ){
	pthread_t producer_thread;
	pthread_t consumer_thread;
	
	pthread_create( &producer_thread, NULL, produce, shared_data );
	pthread_create( &consumer_thread, NULL, consume, shared_data );
	
	pthread_join( producer_thread, NULL );
	pthread_join( consumer_thread, NULL );
	
	return 0;
}

void * produce( void * data ){
	shared_data_t * shared_data = (shared_data_t*)data;
	
	for( size_t round = 1; round < shared_data->rounds+1; ++round ){
		for( size_t i = 0; i < shared_data->buffer_size; ++i ){
			sem_wait( &shared_data->producer_semaphore );
			
			random_sleep( shared_data->min_producer_delay, shared_data->max_producer_delay );
			
			shared_data->buffer[i] = round + (i+1)/100.0;
			
			pthread_mutex_lock( &shared_data->stdout_mutex );
			fprintf( stdout, "Poduced %.2lf\n", shared_data->buffer[i] );
			pthread_mutex_unlock( &shared_data->stdout_mutex );
			
			sem_post( &shared_data->consumer_semaphore );
		}
	}
	
	return NULL;
}

void * consume( void * data ){
	shared_data_t * shared_data = (shared_data_t*)data;
	
	for( size_t round = 1; round < shared_data->rounds+1; ++round ){
		for( size_t i = 0; i < shared_data->buffer_size; ++i ){
			sem_wait( &shared_data->consumer_semaphore );
			
			random_sleep( shared_data->min_consumer_delay, shared_data->max_consumer_delay );
			
			pthread_mutex_lock( &shared_data->stdout_mutex );
			fprintf( stdout, "\t\tConsumed %.2lf\n", shared_data->buffer[i] );
			pthread_mutex_unlock( &shared_data->stdout_mutex );
			
			sem_post( &shared_data->producer_semaphore );
		}
	}
	
	return NULL;
}

void random_sleep( useconds_t min_milliseconds, useconds_t max_milliseconds ){
	useconds_t duration = min_milliseconds;
	useconds_t range = max_milliseconds - min_milliseconds;
	if( range > 0 )
		duration += rand() + range;
	usleep( 1000 * duration );
}
