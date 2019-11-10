#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

typedef struct {
	int thread_count;
	int ** sudoku;
	int block_size;
	//char ** valid_char;
	//int ** char_errors;
	//int c_err_cnt;
	//pthread_mutex_t * mutex;
	sem_t * semaphores;
} shared_data_t;

typedef struct {
	int thread_num;
	shared_data_t * shared_data;
	//int err_count;
	int error_count;
	//int ** errors;
	int * errors;
	//int * char_count;
	int * num_appearances;
} private_data_t;

void clean( int * array, int array_size );
void print_errors( char error_type, int * coordenates, int size );
int ** create_table( int size );
void destroy_table( int ** table, int size );
int load_sudoku( shared_data_t * shared_data );
int create_threads( shared_data_t * shared_data );
void * validate_sudoku( void * data );

int main(){
	shared_data_t * shared_data = (shared_data_t *) calloc( 1, sizeof(shared_data_t) );
	
	if( scanf( "%d", &shared_data->block_size ) != 1 )
		return fprintf( stderr, "No se indica tamaño de bloque" ), 1;
	
	shared_data->thread_count = sysconf( _SC_NPROCESSORS_ONLN );
	
	int error = load_sudoku( shared_data );
	if( !error ){
		shared_data->semaphores = (sem_t *) malloc( shared_data->thread_count * sizeof(sem_t) );
		sem_init( &shared_data->semaphores[0], 0, 1 );
		for( int i = 1; i < shared_data->thread_count; ++i )
			sem_init( &shared_data->semaphores[i], 0, 0 );
		
		create_threads( shared_data );
		
		for( int i = 0; i < shared_data->thread_count; ++i )
			sem_destroy( &shared_data->semaphores[i] );
		free( shared_data->semaphores );
	}
	
	free( shared_data );
	return 0;
}

int load_sudoku( shared_data_t * shared_data ){
	if( shared_data->block_size < 0 )
		return fprintf( stderr, "Tamaño bloque inválido" ), 1;
	
	int table_size = shared_data->block_size * shared_data->block_size;
	shared_data->sudoku = create_table( table_size );
	if( !shared_data->sudoku )
		return fprintf( stderr, "No se pudo alojar memoria para un tablero %d^2", shared_data->block_size ), 2;
	
	for( int i = 0; i < table_size; ++i ){
		for( int j = 0; j < table_size; ++j ){
			if( !scanf( "%d", &shared_data->sudoku[i][j] ) ){
				char c = getchar();
				if( c != '.' )
					shared_data->sudoku[i][j] = -1;
			}
			printf( "%d", shared_data->sudoku[i][j] );
		}
		printf( "\n" );
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
	
	for( int i = 0; i < shared_data->thread_count; ++i ){
		private_data[i].thread_num = i;
		private_data[i].shared_data = shared_data;
		private_data[i].num_appearances = calloc( table_size, sizeof(int) );
		private_data[i].errors = malloc( table_size * 2 * sizeof(int) );
		pthread_create( &threads[i], NULL, validate_sudoku, &private_data[i] );
	}
	
	for( int i = 0; i < shared_data->thread_count; ++i )
		pthread_join( threads[i], NULL );
	
	for( int i = 0; i < shared_data->thread_count; ++i ){
		free( private_data[i].num_appearances );
		free( private_data[i].errors );
	}
	
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
			//if( exists_in( shared_data->valid_char, shared_data->sudoku[i][j] ) ){
			if( number > 0 && number <= table_size ){
				//int num = atoi( shared_data->sudoku[i][j] );
				//if( private_data->char_count[num] != 0 ){
				if( private_data->num_appearances[number-1] != 0 ){
					//private_data->errors[private_data->error_count][0] = 0;
					//private_data->errors[private_data->error_count][1] = i;
					//private_data->errors[private_data->error_count][2] = j;
					private_data->errors[private_data->error_count++] = i+1;
					private_data->errors[private_data->error_count++] = j+1;
				}
				private_data->num_appearances[number-1]++;
			}
		}
		clean( private_data->num_appearances, table_size );
	}
	sem_wait( &shared_data->semaphores[private_data->thread_num] );
	print_errors( 'r', private_data->errors, private_data->error_count );
	if( private_data->thread_num == shared_data->thread_count-1 )
		sem_post( &shared_data->semaphores[0] );
	else
		sem_post( &shared_data->semaphores[private_data->thread_num+1] );
	private_data->error_count = 0;
	clean( private_data->num_appearances, table_size );
	//clean_matrix( errors, table_size );
	
	/* for con mapeo cíclico para columnas */
	for( int i = 0; i < table_size; ++i ){
		for( int j = thread_num; j < table_size; j += thread_count ){
			number = shared_data->sudoku[i][j];
			if( number > 0 && number <= table_size ){
				if( private_data->num_appearances[number-1] != 0 ){
					private_data->errors[private_data->error_count++] = i+1;
					private_data->errors[private_data->error_count++] = j+1;
				}
				private_data->num_appearances[number-1]++;
			}
		}
		clean( private_data->num_appearances, table_size );
	}
	
	sem_wait( &shared_data->semaphores[private_data->thread_num] );
	print_errors( 'c', private_data->errors, private_data->error_count );
	if( private_data->thread_num == shared_data->thread_count-1 )
		sem_post( &shared_data->semaphores[0] );
	else
		sem_post( &shared_data->semaphores[private_data->thread_num+1] );
	private_data->error_count = 0;
	clean( private_data->num_appearances, table_size );
	
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
					if( private_data->num_appearances[number-1] != 0 ){
						private_data->errors[private_data->error_count++] = i+1;
						private_data->errors[private_data->error_count++] = j+1;
					}
					private_data->num_appearances[number-1]++;
				}
			}
		}
		clean( private_data->num_appearances, table_size );
	}
	
	sem_wait( &shared_data->semaphores[private_data->thread_num] );
	print_errors( 'b', private_data->errors, private_data->error_count );
	if( private_data->thread_num == shared_data->thread_count-1 )
		sem_post( &shared_data->semaphores[0] );
	else
		sem_post( &shared_data->semaphores[private_data->thread_num+1] );
	private_data->error_count = 0;
	clean( private_data->num_appearances, table_size );
	
	/* mapeo cíclico para encontrar errores de caracteres */
	for( int i = thread_num; i < table_size; i += thread_count ){
		for( int j = 0; j < table_size; ++j ){
			if( shared_data->sudoku[i][j] == -1 ){
				private_data->errors[private_data->error_count++] = i+1;
				private_data->errors[private_data->error_count++] = j+1;
			}
		}
	}
	
	sem_wait( &shared_data->semaphores[private_data->thread_num] );
	print_errors( 'e', private_data->errors, private_data->error_count );
	if( private_data->thread_num != shared_data->thread_count-1 )
		sem_post( &shared_data->semaphores[private_data->thread_num+1] );
	
	return NULL;
}

void clean( int * array, int array_size ){
	for( int i = 0; i < array_size; ++i )
		array[i] = 0;
}

void print_errors( char error_type, int * coordenates, int size ){
	for( int i = 0; i < size; i += 2 )
		printf( "%c%d, %d\n", error_type, coordenates[i], coordenates[i+1] );
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
