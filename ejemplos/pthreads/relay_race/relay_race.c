#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct {
	sem_t * batons;
	pthread_barrier_t start_barrier;
	useconds_t stage1_duration;
	useconds_t stage2_duration;
	size_t position;
	pthread_mutex_t position_mutex;
	size_t team_count;
} shared_data_t;

typedef struct {
	size_t my_team_num;
	shared_data_t * shared_data;
} private_data_t;

int analize_arguments( int argc, char* argv[], shared_data_t * shared_data );
int create_threads( shared_data_t * shared_data );
void * start_race( void * data );
void * finish_race( void * data );

int main( int argc, char* argv[] ){
	shared_data_t * shared_data = (shared_data_t*) calloc( 1, sizeof(shared_data_t) );

	int error = analize_arguments( argc, argv, shared_data );
	if( !error ){
		shared_data->batons = (sem_t*) calloc( shared_data->team_count, sizeof(sem_t) );
		if( shared_data->batons ){
			for( size_t i = 0; i < shared_data->team_count; ++i )
				sem_init( &shared_data->batons[i], 0, 0 );
			
			pthread_barrier_init( &shared_data->start_barrier, NULL, shared_data->team_count );
			shared_data->position = 0;
			pthread_mutex_init( &shared_data->position_mutex, NULL );
			
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
			pthread_mutex_destroy( &shared_data->position_mutex );
			pthread_barrier_destroy( &shared_data->start_barrier );
			free( shared_data->batons );
		} else {
			fprintf( stderr, "error: could not allocate memory for %zu batons\n", shared_data->team_count );
			error = 2;
		}
	}
	
	free( shared_data );
	return 0;
}

int analize_arguments( int argc, char* argv[], shared_data_t * shared_data ){
	if( argc == 4 ){
		if( sscanf( argv[1], "%zu", &shared_data->team_count ) != 1 || shared_data->team_count == 0 )
			return (void)fprintf( stderr, "invalid team count: %s\n", argv[1] ), 1;
		if( sscanf( argv[2], "%u", &shared_data->stage1_duration ) != 1 )
			return (void)fprintf( stderr, "invalid stage1 duration: %s\n", argv[2] ), 2;
		if( sscanf( argv[3], "%u", &shared_data->stage2_duration ) != 1 )
			return (void)fprintf( stderr, "invalid stage2 duration: %s\n", argv[3] ), 3;
	} else {
		fprintf( stderr, "relay_race teams stage1_time stage2_time\n");
		return 1;
	}
	return 0;
}

int create_threads( shared_data_t * shared_data ){
	pthread_t * threads = (pthread_t*) malloc( 2 * shared_data->team_count * sizeof(pthread_t) );
	if( !threads )
		return fprintf( stderr, "error: could not allocate memory for %zu threads\n", 2 * shared_data->team_count ), 1;
	
	private_data_t * private_data = (private_data_t*) calloc( 2 * shared_data->team_count, sizeof(private_data_t) );
	if( !private_data )
		return fprintf( stderr, "error: could not allocate private memory for %zu threads\n", 2 * shared_data->team_count ), 3;
	
  #if INVERTED_TEAM_ORDER
	for( size_t i = shared_data->team_count - 1; i < shared_data->team_count; --i ){
  #else 
	for( size_t i = 0; i < shared_data->team_count; ++i ){
  #endif
		private_data[i].my_team_num = i;
		private_data[i].shared_data = shared_data;
		pthread_create( &threads[i], NULL, start_race, &private_data[i] );
	}
	
  #if INVERTED_TEAM_ORDER
	for( size_t i = 2 * shared_data->team_count - 1; i >= shared_data->team_count; --i ){
  #else
	for( size_t i = shared_data->team_count; i < 2 * shared_data->team_count; ++i ){
  #endif
		private_data[i].my_team_num = i - shared_data->team_count;
		private_data[i].shared_data = shared_data;
		pthread_create( &threads[i], NULL, finish_race, &private_data[i] );
	}
	
	for( size_t index = 0; index < 2 * shared_data->team_count; ++index )
		pthread_join( threads[index], NULL );
	
	free( private_data );
	free( threads );
	return 0;
}

void * start_race( void * data ){
	private_data_t * private_data = (private_data_t*)data;
	shared_data_t * shared_data = private_data->shared_data;
	
	pthread_barrier_wait( &shared_data->start_barrier );
	
	usleep( 1000 * shared_data->stage1_duration );
	
	sem_post( &shared_data->batons[private_data->my_team_num] );
	
	return NULL;
}

void * finish_race( void * data ){
	private_data_t * private_data = (private_data_t*)data;
	shared_data_t * shared_data = private_data->shared_data;
	
	sem_wait( &shared_data->batons[private_data->my_team_num] );
	
	usleep( 1000 * shared_data->stage2_duration );
	
	pthread_mutex_lock( &shared_data->position_mutex );
	if( shared_data->position++ <= 2 )
		printf( "Place %zu: team %zu\n", shared_data->position, private_data->my_team_num + 1 );
	pthread_mutex_unlock( &shared_data->position_mutex );
	
	return NULL;
}
