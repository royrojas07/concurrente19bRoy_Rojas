#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static size_t worker_count = 3;
static size_t number_count = 3;
static size_t* numbers = NULL;
static size_t current_step = 0;
static size_t max_steps = 10;
static pthread_t* workers = NULL;
static pthread_barrier_t barrier;

void* calculate(void* data)
{
	const size_t my_id = (size_t)data;
	size_t next;
	size_t prev;

	while( current_step < max_steps ){
		pthread_barrier_wait(&barrier);
		for ( size_t i = my_id; i < number_count; i += worker_count )
		{
			if( numbers[i] > 1 ){
				next = (i + 1) % worker_count;
				prev = (size_t)((long long)i - 1) % worker_count;
				
				if ( numbers[i] % 2 == 0 )
					numbers[i] /= 2;
				else
					numbers[i] = numbers[prev] * numbers[i] + numbers[next];
			}
		}
		
		if ( my_id == 0 )
			++current_step;

		pthread_barrier_wait(&barrier);
	}
	return NULL;
}

int main()
{
	scanf("%zu %zu %zu\n", &number_count, &max_steps, &worker_count);

	numbers = malloc( number_count * sizeof(size_t) );
	for ( size_t index = 0; index < number_count; ++index )
		scanf("%zu", &numbers[index]);

	pthread_barrier_init(&barrier, NULL, (unsigned)worker_count);

	workers = malloc(worker_count * sizeof(pthread_t));
	for ( size_t index = 0; index < worker_count; ++index )
		pthread_create(&workers[index], NULL, calculate, (void*)index);

	for ( size_t index = 0; index < worker_count; ++index )
		pthread_join(workers[index], NULL);

	pthread_barrier_destroy(&barrier);

	if ( current_step > max_steps )
		printf("No converge in %zu steps\n", max_steps);
	else
		printf("Converged in %zu steps\n", current_step);

	return 0;
}
