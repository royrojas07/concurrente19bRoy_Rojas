#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#include "array_rwlock.h"


typedef struct array_rwlock
{
	void** elements;
	size_t capacity;
	size_t count;
	pthread_rwlock_t rwlock;
} array_rwlock_t;

// Private declarations
size_t array_rwlock_find_first_private(array_rwlock_t* array, const void* element, size_t start_pos);


array_rwlock_t* array_rwlock_create(size_t capacity)
{
	assert(capacity);
	
	array_rwlock_t* array = calloc(1, sizeof(array_rwlock_t));
	if ( array == NULL )
		return NULL;

	array->capacity = capacity;
	array->count = 0;

	pthread_rwlock_init( &array->rwlock, NULL );

	array->elements = (void**)malloc( capacity * sizeof(void*) );
	if ( array->elements == NULL )
		return (void)free(array), NULL;
	
	return array;
}

void array_rwlock_destroy(array_rwlock_t* array)
{
	assert(array);

	pthread_rwlock_destroy(&array->rwlock);

	free(array->elements);
	free(array);
}

int array_rwlock_increase_capacity(array_rwlock_t* array)
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

int array_rwlock_decrease_capacity(array_rwlock_t* array)
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

size_t array_rwlock_get_count(array_rwlock_t* array)
{
	assert(array);

	pthread_rwlock_rdlock( &array->rwlock );
	size_t result = array->count;
	pthread_rwlock_unlock( &array->rwlock );
	return result;
}

void* array_rwlock_get_element(array_rwlock_t* array, size_t index)
{
	assert(array);
	assert( index < array_rwlock_get_count(array) );

	pthread_rwlock_rdlock( &array->rwlock );
	void* result = array->elements[index];
	pthread_rwlock_unlock( &array->rwlock );
	return result;
}

int array_rwlock_append(array_rwlock_t* array, void* element)
{
	assert(array);

	pthread_rwlock_wrlock( &array->rwlock );
	if ( array->count == array->capacity )
		if ( array_rwlock_increase_capacity(array) )
			return (void)pthread_rwlock_unlock( &array->rwlock ), -1;

	assert( array->count < array->capacity );
	array->elements[array->count++] = element;
	pthread_rwlock_unlock( &array->rwlock );
	return 0; // Success
}

size_t array_rwlock_find_first(array_rwlock_t* array, const void* element, size_t start_pos)
{
	assert( array );
	
	pthread_rwlock_rdlock( &array->rwlock );
	size_t result = array_rwlock_find_first_private(array, element, start_pos);
	pthread_rwlock_unlock( &array->rwlock );
	return result;
}

size_t array_rwlock_find_first_private(array_rwlock_t* array, const void* element, size_t start_pos)
{
	assert( array );

	for ( size_t index = start_pos; index < array->count; ++index )
		if ( array->elements[index] == element )
			return index;

	return array_rwlock_not_found;
}

int array_rwlock_remove_first(array_rwlock_t* array, const void* element, size_t start_pos)
{
	assert( array );

	pthread_rwlock_wrlock( &array->rwlock );
	size_t index = array_rwlock_find_first_private(array, element, start_pos);
	if ( index == array_rwlock_not_found )
		return (void)pthread_rwlock_unlock( &array->rwlock ), -1;

	for ( --array->count; index < array->count; ++index )
		array->elements[index] = array->elements[index + 1];
	if ( array->count == array->capacity / 10 )
		array_rwlock_decrease_capacity(array);

	pthread_rwlock_unlock( &array->rwlock );
	return 0; // Removed
}
