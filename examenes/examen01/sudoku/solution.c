#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
	int thread_count;
	int ** sudoku;
	int block_size;
	pthread_barrier_t barrier;
	int ** errors;
	int error_count;
} shared_data_t;

typedef struct {
	int thread_num;
	shared_data_t * shared_data;
	int * num_appearances;
} private_data_t;

void clean_vector( int * vector, int vector_size );
void clean_matrix( int ** matrix, int size, int thread_num, int thread_count );
int print_errors( char error_type, int ** errors, int size );
int ** create_table( int size );
void destroy_table( int ** table, int size );
int load_sudoku( shared_data_t * shared_data );
int create_threads( shared_data_t * shared_data );
void * validate_sudoku( void * data );

int main(){
	shared_data_t * shared_data = (shared_data_t *) calloc( 1, sizeof(shared_data_t) );
	
	if( scanf( "%d", &shared_data->block_size ) != 1 )
		return fprintf( stderr, "No se indica tamaño de bloque" ), 1;
	else if( shared_data->block_size < 0 )
		return fprintf( stderr, "Tamaño bloque inválido" ), 2;
	
	shared_data->thread_count = sysconf( _SC_NPROCESSORS_ONLN );
	
	int error = load_sudoku( shared_data );
	if( !error ){
		pthread_barrier_init( &shared_data->barrier, NULL, shared_data->thread_count );
		
		create_threads( shared_data );
		if( !shared_data->error_count )
			printf( "valid\n" );
		
		pthread_barrier_destroy( &shared_data->barrier );
	}
	
	free( shared_data );
	return 0;
}

int load_sudoku( shared_data_t * shared_data ){
	int table_size = shared_data->block_size * shared_data->block_size;
	shared_data->sudoku = create_table( table_size );
	if( !shared_data->sudoku )
		return fprintf( stderr, "No se pudo alojar memoria para un tablero %d^2", table_size ), 2;
	
	for( int i = 0; i < table_size; ++i ){
		for( int j = 0; j < table_size; ++j ){
			if( !scanf( "%d", &shared_data->sudoku[i][j] ) ){
				char c = getchar();
				if( c != '.' )
					shared_data->sudoku[i][j] = -1;
			}
		}
	}
	return 0;
}

int create_threads( shared_data_t * shared_data ){
	pthread_t * threads = (pthread_t *) malloc( shared_data->thread_count * sizeof(pthread_t) );
	if( threads == NULL )
		return fprintf( stderr, "No se pudieron crear tal cantidad de threads" ), 1;
	
	private_data_t * private_data = calloc( shared_data->thread_count, sizeof(private_data_t) );
	if( !private_data )
		return fprintf( stderr, "No se pudo alojar memoria privada para tal cantidad de threads" ),
			free( threads ), 2;
	
	int table_size = shared_data->block_size * shared_data->block_size;
	shared_data->errors = create_table( table_size );
	if( !shared_data->errors )
		return fprintf( stderr, "No se pudo alojar memoria para un tablero %d^2", table_size ), 2;
	
	for( int i = 0; i < shared_data->thread_count; ++i ){
		private_data[i].thread_num = i;
		private_data[i].shared_data = shared_data;
		private_data[i].num_appearances = calloc( table_size, sizeof(int) );
		pthread_create( &threads[i], NULL, validate_sudoku, &private_data[i] );
	}
	
	for( int i = 0; i < shared_data->thread_count; ++i )
		pthread_join( threads[i], NULL );
	
	for( int i = 0; i < shared_data->thread_count; ++i )
		free( private_data[i].num_appearances );
	
	destroy_table( shared_data->errors, table_size );
	destroy_table( shared_data->sudoku, table_size );
	free( private_data );
	free( threads );
	return 0;
}

void * validate_sudoku( void * data ){
	private_data_t * private_data = (private_data_t *)data;
	shared_data_t * shared_data = private_data->shared_data;
	
	int table_size = shared_data->block_size * shared_data->block_size;
	int thread_count = shared_data->thread_count;
	int thread_num = private_data->thread_num;
	int block_size = shared_data->block_size;
	int number;
	
	/* for con mapeo cíclico para filas */
	for( int i = thread_num; i < table_size; i += thread_count ){
		for( int j = 0; j < table_size; ++j ){
			number = shared_data->sudoku[i][j];
			if( number > 0 && number <= table_size ){
				if( private_data->num_appearances[number-1] != 0 )
					shared_data->errors[i][j] = 1;
				private_data->num_appearances[number-1]++;
			}
		}
		clean_vector( private_data->num_appearances, table_size );
	}
	
	pthread_barrier_wait( &shared_data->barrier ); //espera a que todos recorran el sudoku
	if( private_data->thread_num == 0 )
		shared_data->error_count = print_errors( 'r', shared_data->errors, table_size );
	pthread_barrier_wait( &shared_data->barrier ); //espera a que thread 0 imprima errores
	clean_matrix( shared_data->errors, table_size, private_data->thread_num, shared_data->thread_count );
	pthread_barrier_wait( &shared_data->barrier ); //espera a que todos limpien matriz
	
	/* for con mapeo cíclico para columnas */
	for( int j = thread_num; j < table_size; j += thread_count ){
		for( int i = 0; i < table_size; ++i ){
			number = shared_data->sudoku[i][j];
			if( number > 0 && number <= table_size ){
				if( private_data->num_appearances[number-1] != 0 )
					shared_data->errors[i][j] = 1;
				private_data->num_appearances[number-1]++;
			}
		}
		clean_vector( private_data->num_appearances, table_size );
	}
	
	pthread_barrier_wait( &shared_data->barrier );
	if( private_data->thread_num == 0 )
		shared_data->error_count += print_errors( 'c', shared_data->errors, table_size );
	pthread_barrier_wait( &shared_data->barrier );
	clean_matrix( shared_data->errors, table_size, private_data->thread_num, shared_data->thread_count );
	pthread_barrier_wait( &shared_data->barrier );
	
	int initial_row;
	int initial_column;
	/* for con mapeo cíclico para bloques */
	for( int k = thread_num; k < table_size; k += thread_count ){
		initial_row = k - k % block_size;
		for( int i = initial_row; i < initial_row + block_size; ++i ){
			initial_column = (k % block_size)*block_size;
			for( int j = initial_column; j < initial_column + block_size; ++j ){
				number = shared_data->sudoku[i][j];
				if( number > 0 && number <= table_size ){
					if( private_data->num_appearances[number-1] != 0 )
						shared_data->errors[i][j] = 1;
					private_data->num_appearances[number-1]++;
				}
			}
		}
		clean_vector( private_data->num_appearances, table_size );
	}
	
	pthread_barrier_wait( &shared_data->barrier );
	if( private_data->thread_num == 0 )
		shared_data->error_count += print_errors( 'b', shared_data->errors, table_size );
	pthread_barrier_wait( &shared_data->barrier );
	clean_matrix( shared_data->errors, table_size, private_data->thread_num, shared_data->thread_count );
	pthread_barrier_wait( &shared_data->barrier );
	
	/* mapeo cíclico para encontrar errores de caracteres */
	for( int i = thread_num; i < table_size; i += thread_count ){
		for( int j = 0; j < table_size; ++j ){
			if( shared_data->sudoku[i][j] == -1 ){
				shared_data->errors[i][j] = 1;
			}
		}
	}
	
	pthread_barrier_wait( &shared_data->barrier );
	if( private_data->thread_num == 0 )
		shared_data->error_count += print_errors( 'e', shared_data->errors, table_size );
	
	return NULL;
}

void clean_vector( int * vector, int vector_size ){
	for( int i = 0; i < vector_size; ++i )
		vector[i] = 0;
}

void clean_matrix( int ** matrix, int size, int thread_num, int thread_count ){
	for( int i = thread_num; i < size; i += thread_count ){
		for( int j = 0; j < size; ++j )
			matrix[i][j] = 0;
	}
}

int print_errors( char error_type, int ** errors, int size ){
	int error_count = 0;
	for( int i = 0; i < size; ++i ){
		for( int j = 0; j < size; ++j ){
			if( errors[i][j] ){
				printf( "%c%d, %d\n", error_type, i+1, j+1 );
				++error_count;
			}
		}
	}
	return error_count;
}

int ** create_table( int size ){
	int ** table = malloc( size * sizeof(int *) );
	if( !table )
		return NULL;
	
	for( int i = 0; i < size; ++i ){
		table[i] = calloc( size, sizeof(int) );
		if( !table[i] )
			return destroy_table( table, size ), NULL;
	}
	
	return table;
}

void destroy_table( int ** table, int size ){
	for( int i = 0; i < size; ++i )
		free( table[i] );
	free( table );
}
