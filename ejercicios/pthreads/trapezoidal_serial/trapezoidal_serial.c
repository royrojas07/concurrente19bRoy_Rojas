#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long double trapezoidal_area( long double func(), long double a, long double b, int n );
long double parabola_function( long double x );

int main( int argc, char * argv[] ){
	if( argc < 4 )
		return fprintf( stderr, "Argumentos inválidos. Los "
			"argumentos son:\n./programa lim_a lim_b cant_trapezoides\n" ), 1;

	long double l_lim = strtod( argv[1], NULL );
	long double r_lim = strtod( argv[2], NULL );
	int trapezoids = atoi( argv[3] );

	if( l_lim > r_lim || l_lim == r_lim )
		return fprintf( stderr, "Los valores límites del rango son inválidos\n" ), 2;
	else if( trapezoids <= 0 )
		return fprintf( stderr, "La cantidad de trapeziodes es inválida\n" ), 3;

	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	long double area_down_curve = trapezoidal_area( parabola_function, l_lim, r_lim, trapezoids );

	struct timespec finish_time;
	clock_gettime(CLOCK_MONOTONIC, &finish_time);

	printf( "El área bajo la curva es: %.0Lf\n", area_down_curve );

	long double elapsed_seconds = finish_time.tv_sec - start_time.tv_sec
		+ 1e-9 * (finish_time.tv_nsec - start_time.tv_nsec);

	printf( "Execution time %.9Lfs\n", elapsed_seconds );

	return 0;
}

long double trapezoidal_area( long double func(), long double a, long double b, int n ){
	long double area;
	long double subinterval_length = ( b - a )/(long double)n;
	long double bases_sum = 0;

	for( int k = 1; k <= n; ++k ){
		bases_sum += func( a + ((long double)(k-1))*subinterval_length )
			+ func( a + ((long double)k)*subinterval_length );
	}
	
	area = 0.5 * subinterval_length * bases_sum;
	return area;
}

long double parabola_function( long double x ){
	long double y = x*x + 0*x + 0;
	return y;
}
