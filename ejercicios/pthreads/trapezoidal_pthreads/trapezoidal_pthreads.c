#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
	int thread_count;
	double l_lim;
	double r_lim;
	int trapezoids;
	double subinterval_length;
	double area_down_curve;
} shared_data_t;

typedef struct {
	int thread_num;
	double bases_sum;
	shared_data_t * shared_data;
} private_data_t;

int create_threads( shared_data_t * shared_data );
void * trapezoidal_area( void * data );
double parabola_function( double x );
void calculate_area( shared_data_t * shared_data, private_data_t * private_data );

int main( int argc, char * argv[] ){
	shared_data_t * shared_data = (shared_data_t *) calloc( 1, sizeof(shared_data_t) );

	if( argc < 5 )
		return fprintf( stderr, "Argumentos inválidos. Los "
			"argumentos son:\n./programa lim_a lim_b cant_trapezoides cant_threads\n" ), 1;

	shared_data->l_lim = strtod( argv[1], NULL );
	shared_data->r_lim = strtod( argv[2], NULL );
	shared_data->trapezoids = atoi( argv[3] );
	shared_data->thread_count = atoi( argv[4] );

	if( shared_data->l_lim >= shared_data->r_lim )
		return fprintf( stderr, "Los valores límites del rango son inválidos\n" ), 2;
	else if( shared_data->trapezoids <= 0 || shared_data->thread_count < 0 )
		return fprintf( stderr, "La cantidad de trapeziodes o threads es inválida\n" ), 3;

	if( !shared_data->thread_count )
		shared_data->thread_count = sysconf( _SC_NPROCESSORS_ONLN );

	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	int error = create_threads( shared_data );
	if( error )
		return free( shared_data ), error;

	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);

	printf( "El área bajo la curva es: %lf\n", shared_data->area_down_curve );

	double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

	printf( "Execution time %.9lfs\n", elapsed_seconds );

	free( shared_data );
	return 0;
}

void * trapezoidal_area( void * data ){
	private_data_t * private_data = ( private_data_t * )data;
	shared_data_t * shared_data = private_data->shared_data;
	
	private_data->bases_sum = 0;
	
	for( int k = private_data->thread_num+1; k <= shared_data->trapezoids; k += shared_data->thread_count ){
		private_data->bases_sum += parabola_function( shared_data->l_lim + ((double)(k-1))*shared_data->subinterval_length )
			+ parabola_function( shared_data->l_lim + ((double)k)*shared_data->subinterval_length );
	}
	printf("inside trapezoidal area %lf\n", private_data->bases_sum);
	return NULL;
}

int create_threads( shared_data_t * shared_data ){
	pthread_t * threads = (pthread_t *) malloc( shared_data->thread_count * sizeof( shared_data_t ) );
	if( !threads )
		return fprintf( stderr, "No se pudo reservar memoria para %d threads\n", shared_data->thread_count ), 1;

	private_data_t * private_data = (private_data_t *) malloc( shared_data->thread_count * sizeof( private_data_t ) );
	if( !private_data )
		return fprintf( stderr, "No se pudo reservar memoria priavda para %d threads\n", shared_data->thread_count ), 2;

	shared_data->subinterval_length = ( shared_data->r_lim - shared_data->l_lim )/(double)shared_data->trapezoids;
	
	for( int i = 0; i < shared_data->thread_count; ++i ){
		private_data[i].thread_num = i;
		private_data[i].shared_data = shared_data;
		pthread_create( &threads[i], NULL, trapezoidal_area, &private_data[i] );
	}
	printf("%lf\n", private_data[0].bases_sum);
	calculate_area( shared_data, private_data );
	
	for( int i = 0; i < shared_data->thread_count; ++i )
		pthread_join( threads[i], NULL );

	free( private_data );
	free( threads );
	return 0;
}

void calculate_area( shared_data_t * shared_data, private_data_t * private_data ){
	double bases_sum = 0;
	for( int i = 0; i < shared_data->thread_count; ++i )
		bases_sum += private_data[i].bases_sum;
	shared_data->area_down_curve = 0.5 * shared_data->subinterval_length * bases_sum;
}

double parabola_function( double x ){
	double y = x*x + 0*x + 0;
	return y;
}
