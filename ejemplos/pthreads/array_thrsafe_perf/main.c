#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "array_mutex.h"
#include "array_rwlock.h"

typedef struct
{
	size_t initial_element_count;
	size_t operation_count;
	double insertion_percent;
	double deletion_percent;
	double searching_percent;
	size_t worker_count;
	size_t tested_array;
	array_mutex_t* mutex_array;
	array_rwlock_t* rwlock_array;
} shared_t;

typedef struct
{
	size_t worker_id;
	shared_t* shared;
} thread_data_t;

void print_array(const char* name, array_mutex_t* array);
int test_arrays(shared_t* shared);
void* test_array(void* data);

static const char* const usage_text =
	"usage: array_thrsafe_perf #elements #operations %%insertions %%deletions %%searches #workers array\n";

int main(int argc, char* argv[])
{
	// All arguments are required
	if ( argc != 8 )
		return (void)printf(usage_text), 1;

	// Extract the arguments
	shared_t shared;
	shared.initial_element_count = strtoull(argv[1], NULL, 10);
	shared.operation_count = strtoull(argv[2], NULL, 10);
	shared.insertion_percent = strtod(argv[3], NULL);
	shared.deletion_percent = strtod(argv[4], NULL);
	shared.searching_percent = strtod(argv[5], NULL);
	shared.worker_count = strtoull(argv[6], NULL, 10);

	if ( strcmp(argv[7], "mutex") == 0 )
		shared.tested_array = 1;
	else if ( strcmp(argv[7], "rwlock") == 0 )
		shared.tested_array = 2;
	else
		return (void)fprintf(stderr, "error: unknown array: %s", argv[7]), 2;

	shared.mutex_array = NULL;
	shared.rwlock_array = NULL;

	// If we have to thes the mutex array
	if ( shared.tested_array == 1 )
	{
		//  Create the mutex array
		shared.mutex_array = array_mutex_create(shared.initial_element_count);

		// Append the intial elements
		for ( size_t current = 0; current < shared.initial_element_count; ++current )
			array_mutex_append( shared.mutex_array, (void*)(current) );
	}
	else
	{
		shared.rwlock_array = array_rwlock_create(shared.initial_element_count);
		for ( size_t current = 0; current < shared.initial_element_count; ++current )
			array_rwlock_append( shared.rwlock_array, (void*)(current) );
	}

	// Create and test the performance of the thread-safe arrays
	int result = test_arrays(&shared);

	// Destroy the arrays
	if ( shared.tested_array == 1 )
		array_mutex_destroy(shared.mutex_array);
	else
		array_rwlock_destroy(shared.rwlock_array);

	return result;
}

int test_arrays(shared_t* shared)
{
	// Init pseudo-random number generator
	srand( (unsigned)((unsigned)time(NULL) + (unsigned)clock()) );

	// Create threads and their data
	pthread_t* threads = (pthread_t*) malloc( shared->worker_count * sizeof(pthread_t) );
	thread_data_t* thread_data = (thread_data_t*) malloc( shared->worker_count * sizeof(thread_data_t) );

	// Start the asked number of threads
	for ( size_t current = 0; current < shared->worker_count; ++current )
	{
		thread_data[current].worker_id = current;
		thread_data[current].shared = shared;
		pthread_create( threads + current, NULL, test_array, thread_data + current );
	}

	// Wait for all threads to finish
	for ( size_t current = 0; current < shared->worker_count; ++current )
		pthread_join( threads[current], NULL );

	// Release threads and their data
	free(thread_data);
	free(threads);

//	print_array("array1", mutex_array);
//	print_array("array2", rwlock_array);

	return 0;
}

void* test_array(void* data)
{
	// Unpack the data
	thread_data_t* thread_data = (thread_data_t*)data;
	shared_t* shared = thread_data->shared;

	// Slice the operations among the threads
	size_t my_operations = shared->operation_count / shared->worker_count
		+ (thread_data->worker_id < shared->operation_count % shared->worker_count);

	// Execute the asked number of operations
	for ( size_t operation = 0; operation < my_operations; ++operation )
	{
		// Get a random number to insert, find, or delete from array
		size_t element = rand() % 100;

		// Get a random percent to choose the operation to execute
		size_t random_op = (size_t)rand() % shared->operation_count;

		// Execute the operation according to the percents
		if ( random_op < shared->insertion_percent * shared->operation_count )
		{
			// Insert an element into the tested array
			shared->tested_array == 1
				? array_mutex_append( shared->mutex_array, (void*)(element) )
				: array_rwlock_append( shared->rwlock_array, (void*)(element) );
		}
		else if ( random_op < shared->deletion_percent * shared->operation_count )
		{
			// Remove an element in the tested array
			shared->tested_array == 1
				? array_mutex_remove_first(shared->mutex_array, (void*)(element), 0)
				: array_rwlock_remove_first(shared->rwlock_array, (void*)(element), 0);
		}
		else
		{
			// Search an element in the tested array
			shared->tested_array == 1
				? array_mutex_find_first(shared->mutex_array, (void*)(element), 0)
				: array_rwlock_find_first(shared->rwlock_array, (void*)(element), 0);
		}
	}

	return NULL;
}

void print_array(const char* name, array_mutex_t* array)
{
	printf("%s: %zu elements\n", name, array_mutex_get_count(array));
	fflush(stdout);
}
