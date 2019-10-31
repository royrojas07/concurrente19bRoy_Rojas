#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

typedef struct {
	int thread_count;
	double l_lim;
	double r_lim;
	int trapezoids;
	double subinterval_length;
} shared_data_t;

typedef struct {
	int thread_num;
	double bases_sum;
	shared_data_t * shared_data;
} private_data_t;

double trapezoidal_area( double func(), double a, double b, int n );
double parabola_function( double x );

int main( int argc, char * argv[] ){
	shared_data_t * shared_data = (shared_data_t *) calloc( 1, sizeof(shared_data_t) );

	if( argc < 4 )
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

	int error = create_threads( shared_data );
	if( error )
		return error;

	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	double area_down_curve = trapezoidal_area( parabola_function, l_lim, r_lim, trapezoids );

	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);

	printf( "El área bajo la curva es: %lf\n", area_down_curve );

	double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

	printf( "Execution time %.9lfs\n", elapsed_seconds );

	free( shared_data );
	return 0;
}

void trapezoidal_area( void * data ){
	private_data_t * private_data = ( private_data_t * )data;
	
	//double area;
	//double subinterval_length = ( b - a )/(double)n;
	//double bases_sum = 0;

	for( int k = private_data->thread_num; k <= private_data->shared_data->thread_count; k += private_data->shared_data->thread_count ){
		private_data->bases_sum += func( a + ((double)(k-1))*subinterval_length )
			+ func( a + ((double)k)*subinterval_length );
	}

	//area = 0.5 * subinterval_length * bases_sum;
}

int create_threads( shared_data_t * shared_data ){
	pthread_t * threads = (pthread_t *) malloc( shared_data->thread_count * sizeof( shared_data_t ) );
	if( !threads )
		return fprintf( stderr, "No se pudo reservar memoria para %d threads\n", shared_data->thread_count ), 1;

	private_data_t * private_data = (private_data_t *) malloc( shared_data->thread_count * sizeof( private_data_t ) );
	if( !private_data )
		return fprintf( stderr, "No se pudo reservar memoria priavda para %d threads\n", shared_data->thread_count ), 2;

	shared_data->subinterval_length = ( r_lim - l_lim )/(double)shared_data->trapezoids;

	for( int i = 0; i < shared_data->thread_count; ++i ){
		private_data[i].thread_num = i;
		private_data[i].shared_data = shared_data;
		create_threads( &threads[i], NULL, trapezoidal_area, &private_data[i] );
	}

	free( private_data );
	free( threads );
	return 0;
}

double parabola_function( double x ){
	double y = x*x + 0*x + 0;
	return y;
}
