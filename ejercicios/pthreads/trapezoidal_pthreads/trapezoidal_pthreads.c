#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
	int thread_count;
	double l_lim;
	double r_lim;
	int trapezoids;
	double subinterval_length;
	double area_down_curve;
	//pthread_mutex_t mutex;
	sem_t * semaphores;
} shared_data_t;

typedef struct {
	int thread_num;
	int entry_cnt;
	shared_data_t * shared_data;
} private_data_t;

int create_threads( shared_data_t * shared_data );
void * trapezoidal_area( void * data );
double parabola_function( double x );

int main( int argc, char * argv[] ){
	shared_data_t * shared_data = (shared_data_t *) calloc( 1, sizeof(shared_data_t) );

	if( argc < 4 )
		return fprintf( stderr, "Argumentos inválidos. Los "
			"argumentos son:\n./programa lim_a lim_b cant_trapezoides cant_threads\n" ), 1;

	shared_data->l_lim = strtod( argv[1], NULL );
	shared_data->r_lim = strtod( argv[2], NULL );
	shared_data->trapezoids = atoi( argv[3] );
	shared_data->thread_count = sysconf( _SC_NPROCESSORS_ONLN );
	if( argc == 5 )
		shared_data->thread_count = atoi( argv[4] );

	if( shared_data->l_lim >= shared_data->r_lim )
		return fprintf( stderr, "Los valores límites del rango son inválidos\n" ), 2;
	else if( shared_data->trapezoids <= 0 || shared_data->thread_count <= 0 )
		return fprintf( stderr, "La cantidad de trapeziodes o threads es inválida\n" ), 3;
	
	//pthread_mutex_init( &shared_data->mutex, NULL );
	
	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	int error = create_threads( shared_data );
	if( error )
		return free( shared_data ), error;
	
	shared_data->area_down_curve *= 0.5 * shared_data->subinterval_length;
	
	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);

	printf( "El área bajo la curva es: %lf\n", shared_data->area_down_curve );

	double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

	printf( "Execution time %.9lfs\n", elapsed_seconds );
	
	//pthread_mutex_destroy( &shared_data->mutex );
	free( shared_data );
	return 0;
}

void * trapezoidal_area( void * data ){
	private_data_t * private_data = ( private_data_t * )data;
	shared_data_t * shared_data = private_data->shared_data;
	
	double bases_sum = 0;
	private_data->entry_cnt = 0;
	for( int k = private_data->thread_num+1; k <= shared_data->trapezoids; k += shared_data->thread_count ){
		bases_sum = parabola_function( shared_data->l_lim + ((double)(k-1))*shared_data->subinterval_length )
			+ parabola_function( shared_data->l_lim + ((double)k)*shared_data->subinterval_length );
		sem_wait( &shared_data->semaphores[private_data->thread_num] );
		shared_data->area_down_curve += bases_sum;
		if( private_data->thread_num != shared_data->thread_count-1 )
			sem_post( &shared_data->semaphores[private_data->thread_num+1] );
		else
			sem_post( &shared_data->semaphores[0] );
	}
	
	/*pthread_mutex_lock( &shared_data->mutex );
	shared_data->area_down_curve += bases_sum;
	pthread_mutex_unlock( &shared_data->mutex );*/
	
	return NULL;
}

int create_threads( shared_data_t * shared_data ){
	pthread_t * threads = (pthread_t *) malloc( shared_data->thread_count * sizeof( shared_data_t ) );
	if( !threads )
		return fprintf( stderr, "No se pudo reservar memoria para %d threads\n", shared_data->thread_count ), 1;

	private_data_t * private_data = (private_data_t *) malloc( shared_data->thread_count * sizeof( private_data_t ) );
	if( !private_data )
		return fprintf( stderr, "No se pudo reservar memoria priavda para %d threads\n", shared_data->thread_count ), 2;
	
	/* * */
	sem_t * sems = (sem_t *) malloc( shared_data->thread_count * sizeof( sem_t ) );
	shared_data->semaphores = sems;
	sem_init( &shared_data->semaphores[0], 0, 1 );
	for( int i = 1; i < shared_data->thread_count; ++i )
		sem_init( &shared_data->semaphores[i], 0, 0 );
	/* * */
	
	shared_data->subinterval_length = ( shared_data->r_lim - shared_data->l_lim )/(double)shared_data->trapezoids;
	
	for( int i = 0; i < shared_data->thread_count; ++i ){
		private_data[i].thread_num = i;
		private_data[i].shared_data = shared_data;
		pthread_create( &threads[i], NULL, trapezoidal_area, &private_data[i] );
	}
	
	for( int i = 0; i < shared_data->thread_count; ++i )
		pthread_join( threads[i], NULL );
	
	free( sems );
	free( private_data );
	free( threads );
	return 0;
}

/*void calculate_area( shared_data_t * shared_data, private_data_t * private_data ){
	double bases_sum = 0;
	for( int i = 0; i < shared_data->thread_count; ++i )
		bases_sum += private_data[i].bases_sum;
	shared_data->area_down_curve = 0.5 * shared_data->subinterval_length * bases_sum;
	printf("inside calculate area %lf\n", private_data->bases_sum);
	printf("inside calculate area %lf\n", private_data[0].bases_sum);
}*/

double parabola_function( double x ){
	double y = x*x + 0*x + 0;
	return y;
}
