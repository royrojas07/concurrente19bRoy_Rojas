#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

void partial_sum( double func(double), double a, double b, int n, double * current_sum );
double parabola_function( double x );

int main( int argc, char * argv[] ){
	if( argc < 4 )
		return fprintf( stderr, "Argumentos inválidos. Los "
			"argumentos son:\n./programa lim_a lim_b trapezoids workers\n" ), 1;

	double l_lim = strtod( argv[1], NULL );
	double r_lim = strtod( argv[2], NULL );
	int trapezoids = atoi( argv[3] );
	int workers = omp_get_max_threads();
	if( argc > 4 )
		workers = atoi( argv[4] );

	if( l_lim > r_lim || l_lim == r_lim )
		return fprintf( stderr, "Los valores límites del rango son inválidos\n" ), 2;
	else if( trapezoids <= 0 )
		return fprintf( stderr, "La cantidad de trapezoides es inválida\n" ), 3;

	double area_down_curve = 0;

	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	#pragma omp parallel num_threads(workers)
	partial_sum( parabola_function, l_lim, r_lim, trapezoids, &area_down_curve );

	//double subinterval_length = ( r_lim - l_lim )/(double)trapezoids;
	//area_down_curve *= 0.5 * subinterval_length;

	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);

	printf( "El área bajo la curva es: %lf\n", area_down_curve );

	double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

	printf( "Execution time %.9lfs\n", elapsed_seconds );

	return 0;
}

void partial_sum( double func(double), double a, double b, int n, double * current_sum ){
	double subinterval_length = ( b - a )/(double)n;
	double partial_sum = 0;

	#pragma omp for
	for( int k = 1; k <= n; ++k ){
		partial_sum += func( a + ((double)(k-1))*subinterval_length )
			+ func( a + ((double)k)*subinterval_length );
	}
	partial_sum *= 0.5 * subinterval_length;

	#pragma omp critical
	*current_sum += partial_sum;
}

double parabola_function( double x ){
	double y = x*x + 0*x + 0;
	return y;
}
