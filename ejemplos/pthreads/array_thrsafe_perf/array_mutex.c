#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#include "array_mutex.h"


typedef struct array_mutex
{
	void** elements;
	size_t capacity;
	size_t count;
	pthread_mutex_t mutex;
} array_mutex_t;

// Private declarations
size_t array_mutex_find_first_private(array_mutex_t* array, const void* element, size_t start_pos);


array_mutex_t* array_mutex_create(size_t capacity)
{
	assert(capacity);
	
	array_mutex_t* array = calloc(1, sizeof(array_mutex_t));
	if ( array == NULL )
		return NULL;

	array->capacity = capacity;
	array->count = 0;

	pthread_mutex_init( &array->mutex, NULL );

	array->elements = (void**)malloc( capacity * sizeof(void*) );
	if ( array->elements == NULL )
		return (void)free(array), NULL;
	
	return array;
}

void array_mutex_destroy(array_mutex_t* array)
{
	assert(array);

	pthread_mutex_destroy(&array->mutex);

	free(array->elements);
	free(array);
}

int array_mutex_increase_capacity(array_mutex_t* array)
{
	assert(array);

	size_t new_capacity = 10 * array->capacity;
	void** new_elements = (void**)realloc( array->elements, new_capacity * sizeof(void*) );
	if ( new_elements == NULL )
		return -1;

	array->capacity = new_capacity;
	array->elements = new_elements;

	return 0; // Success
}

int array_mutex_decrease_capacity(array_mutex_t* array)
{
	assert(array);

	size_t new_capacity = array->capacity / 10;
	if ( new_capacity < 10 )
		return 0;

	void** new_elements = (void**)realloc( array->elements, new_capacity * sizeof(void*) );
	if ( new_elements == NULL )
		return -1;

	array->capacity = new_capacity;
	array->elements = new_elements;

	return 0; // Success
}

size_t array_mutex_get_count(array_mutex_t* array)
{
	assert(array);

	pthread_mutex_lock( &array->mutex );
	size_t result = array->count;
	pthread_mutex_unlock( &array->mutex );
	return result;
}

void* array_mutex_get_element(array_mutex_t* array, size_t index)
{
	assert(array);
	assert( index < array_mutex_get_count(array) );

	pthread_mutex_lock( &array->mutex );
	void* result = array->elements[index];
	pthread_mutex_unlock( &array->mutex );
	return result;
}

int array_mutex_append(array_mutex_t* array, void* element)
{
	assert(array);

	pthread_mutex_lock( &array->mutex );
	if ( array->count == array->capacity )
		if ( array_mutex_increase_capacity(array) )
			return (void)pthread_mutex_unlock( &array->mutex ), -1;

	assert( array->count < array->capacity );
	array->elements[array->count++] = element;
	pthread_mutex_unlock( &array->mutex );
	return 0; // Success
}

size_t array_mutex_find_first(array_mutex_t* array, const void* element, size_t start_pos)
{
	assert( array );
	
	pthread_mutex_lock( &array->mutex );
	size_t result = array_mutex_find_first_private(array, element, start_pos);
	pthread_mutex_unlock( &array->mutex );
	return result;
}

size_t array_mutex_find_first_private(array_mutex_t* array, const void* element, size_t start_pos)
{
	assert( array );

	for ( size_t index = start_pos; index < array->count; ++index )
		if ( array->elements[index] == element )
			return index;

	return array_mutex_not_found;
}

int array_mutex_remove_first(array_mutex_t* array, const void* element, size_t start_pos)
{
	assert( array );

	pthread_mutex_lock( &array->mutex );
	size_t index = array_mutex_find_first_private(array, element, start_pos);
	if ( index == array_mutex_not_found )
		return (void)pthread_mutex_unlock( &array->mutex ), -1;

	for ( --array->count; index < array->count; ++index )
		array->elements[index] = array->elements[index + 1];
	if ( array->count == array->capacity / 10 )
		array_mutex_decrease_capacity(array);

	pthread_mutex_unlock( &array->mutex );
	return 0; // Removed
}
