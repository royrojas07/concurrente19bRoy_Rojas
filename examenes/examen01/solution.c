#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct {
	int thread_count;
	char ** sudoku;
	int block_size;
	char ** valid_char;
	int ** char_errors;
	int c_err_cnt;
	pthread_mutex_t * mutex;
	sem_t * semaphores;
} shared_data_t;

typedef struct {
	int thread_num;
	shared_data_t * shared_data;
	int err_count;
	int ** errors;
	int * char_count;
} private_data_t;

int main( int argc, char * argv[] ){
	shared_data_t * shared_data = (shared_data_t *) calloc( 1, sizeof(shared_data_t) );
	
	if( scanf( "%d", &shared_data->block_size ) != 1 )
		return fprintf( stderr, "No se indica tamaño de bloque" ), 1;
	
	int error = load_sudoku( argv, shared_data );
	if( !error ){
		for( int i = 0; i < shared_data->block_size * shared_data->block_size; ++i )
			shared_data->valid_char[i] = itoa(i);
		
		pthread_mutex_init( shared_data->mutex, NULL );
		sem_init( semaphores[0], 0, 1 );
		for( int i = 1; i < thread_count; ++i )
			sem_init( semaphores[i], 0, 0 );
		
		create_threads( shared_data );
	}
}

int load_sudoku( char * argv[], shared_data_t * shared_data ){
	if( shared_data->block_size < 0 )
		return fprintf( stderr, "Tamaño bloque inválido" ), 1;
	
	shared_data->sudoku = create_table( shared_data->block_size );
	if( !shared_data->sudoku )
		fprintf( stderr, "No se pudo alojar memoria para un tablero %d^2", &shared_data->block_size ), 2;
	
	int table_size = shared_data->block_size * shared_data->block_size;
	for( int i = 1; i <= table_size; ++i ){
		for( int j = 1; j <= table_size; ++j ){
			scanf( "%c", &shared_data->sudoku[i-1][j-1] );
		}
	}
	return 0;
}

int create_threads( shared_data_t * shared_data ){
	shared_data->thread_count = sysconf( _SC_NPROCESSORS_ONLN );
	pthread_t * threads = (pthread_t *) malloc( shared_data->thread_count * sizeof(pthread_t) );
	if( threads == NULL )
		return fprintf( stderr, "No se pudieron crear tal cantidad de threads" ), 1;
	
	private_data_t * private_data = calloc( shared_data->thread_count, sizeof(private_data_t) );
	if( !private_data )
		return fprintf( stderr, "No se pudo alojar memoria privada para tal cantidad de threads" ),
			free( threads ), 2;
	
	for( int i = 0; i < shared_data->thread_count; ++i ){
		private_data[i].thread_num = i;
		private_data[i].shared_data = shared_data;
		pthread_create( &threads[i], NULL, validate_sudoku, &private_data[i] );
	}
	
	for( int i = 0; i < shared_data->thread_count; ++i )
		pthread_join( threads[i], NULL );
	
	free( private_data );
	free( threads );
	return 0;
}

void * validate_sudoku( void * data ){
	private_data_t * private_data = (private_data_t *)data;
	shared_data_t * shared_data = private_data->shared_data;
	
	int table_size = shared_data->block_size * shared_data->block_size;
	/* for con mapeo cíclico para filas */
	for( int i = private_data->thread_num; i < table_size; i += shared_data->thread_count ){
		for( int j = 0; j < table_size; ++j ){
			if( exists_in( shared_data->valid_char, shared_data->sudoku[i][j] ) ){
				int num = atoi( shared_data->sudoku[i][j] );
				if( private_data->char_count[num] != 0 ){
					private_data->errors[private_data->err_count][0] = 0;
					private_data->errors[private_data->err_count][1] = i;
					private_data->errors[private_data->err_count][2] = j;
				}
				private_data->char_cnt[num]++;
			} else {
				pthread_mutex_lock( shared_data->mutex );
				shared_data->char_errors[shared_data->c_err_cnt++][0] = i;
				shared_data->char_errors[shared_data->c_err_cnt][1] = j;
				pthread_mutex_unlock( shared_data->mutex );
			}
		}
	}
	
	sem_wait( &shared_data->semaphores[private_data->thread_num] );
	print_errors( private_data->errors );
	sem_post( &shared_data->semaphores[private_data->thread_num+1] );
	private_data->err_count = 0;
	clean_matrix( errors, table_size );
	
	/* for con mapeo cíclico para columnas */
	for( int i = 0; i < table_size; ++i ){
		for( int j = private_data->thread_num; j < table_size; j += shared_data->thread_count ){
			/* comentarios escritos en el examen */
		}
	}
	
	sem_wait( &shared_data->semaphores[private_data->thread_num] );
	print_errors( private_data->errors );
	sem_post( &shared_data->semaphores[private_data->thread_num+1] );
	private_data->err_count = 0;
	clean_matrix( errors, table_size );
}
