#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <mpi.h>
#include <vector>

typedef struct{
	int rows;
	int cols;
	double ** image;
	int ** mirror;
	int cloud_count;
	std::vector<int> cloud_areas;
	int min;
	int max;
	int init_row;
	int fin_row;
	FILE * file;
} data_t;

void search_clouds( data_t * data );
void expand_cloud( int r, int c, data_t * data );
bool exists( int r, int c, data_t * data );
void print_clouds( std::vector<int> cloud_areas );
double ** read_image( data_t * data );
int ** get_mirror( int begin, int end, int cols );
int my_start( int rank, int D, int W );
void update_last_row( int to_mod, int mod, data_t * data );
void destroy_image( double ** image, int size );
void destroy_mirror( int ** mirror, int size );

int main( int argc, char * argv[] ){
	MPI_Init( &argc, &argv );

	data_t * data = ( data_t * ) calloc( 1, sizeof( data_t ) );
	data->file = fopen( argv[1], "r" );
	fscanf( data->file, "%d", &data->rows );
	fscanf( data->file, "%d", &data->cols );
	data->image = read_image( data );
	data->min = atoi( argv[2] );
	data->max = atoi( argv[3] );

	int rank = -1;
	int proc_cnt = -1;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &proc_cnt );

	data->init_row = my_start( rank, data->rows, proc_cnt );
	data->fin_row = my_start( rank+1, data->rows, proc_cnt );
	data->mirror = get_mirror( data->init_row, data->fin_row, data->cols );

	search_clouds( data );

	int prev = rank-1 < 0 ? proc_cnt-1 : rank-1;
	int next = rank+1 == proc_cnt ? 0 : rank+1;
	int prev_areas_count = 0;
	std::vector<int> prev_areas;
	int * prev_last_row = ( int * ) malloc( data->cols * sizeof( int ) );
	int range = data->fin_row - data->init_row;

	if( rank != 0 ){
		MPI_Recv( &prev_areas_count, 1, MPI::INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		prev_areas.resize( prev_areas_count );
		MPI_Recv( &prev_areas[0], prev_areas_count, MPI::INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &prev_last_row, data->cols, MPI::INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		for( int i = 0; i < data->cloud_areas.size(); ++i ){
			for( int j = 0; j < data->cols; ++j ){
				if( data->mirror[range-1][j] == i+1 && prev_last_row[j] != -1 ){
					prev_areas[prev_last_row[j]-1] += data->cloud_areas[i];
					if( i+1 != prev_last_row[j] )
						update_last_row( i+1, prev_last_row[j], data );
					j = data->cols;
				} else if( j == data->cols-1 ){
					prev_areas.push_back( data->cloud_areas[i] );
					if( i+1 != prev_areas.size() )
						update_last_row( i+1, prev_areas.size(), data );
				}
			}
		}

		if( rank != proc_cnt-1 ){
			int areas_count = prev_areas.size();
			MPI_Send( &areas_count, 1, MPI::INT, next, 0, MPI_COMM_WORLD );
			MPI_Send( &prev_areas[0], areas_count, MPI::INT, next, 0, MPI_COMM_WORLD );
			MPI_Send( &data->mirror[range-1][0], data->cols, MPI::INT, next, 0, MPI_COMM_WORLD );
		}
		else
			print_clouds( prev_areas );
	} else {
		int areas_count = data->cloud_areas.size();
		MPI_Send( &areas_count, 1, MPI::INT, next, 0, MPI_COMM_WORLD );
		MPI_Send( &data->cloud_areas[0], areas_count, MPI::INT, next, 0, MPI_COMM_WORLD );
		MPI_Send( &data->mirror[range-1][0], data->cols, MPI::INT, next, 0, MPI_COMM_WORLD );
	}

	destroy_mirror( data->mirror, range );
	destroy_image( data->image, data->cols );

	return 0;
}

void search_clouds( data_t * data ){
	for( int r = data->init_row; r < data->fin_row; ++r ){
		for( int c = 0; c < data->cols; ++c ){
			if( data->mirror[r][c] == 0 ){
				if( data->min <= data->image[r][c] && data->max >= data->image[r][c] ){
					++data->cloud_count;
					data->cloud_areas.push_back(0);
					expand_cloud( r, c, data );
				}
				else
					data->mirror[r][c] = -1;
			}
		}
	}
}

void expand_cloud( int r, int c, data_t * data ){
	if( exists( r, c, data ) && data->mirror[r][c] == 0 &&
		data->min <= data->image[r][c] && data->max >= data->image[r][c] ){
			data->mirror[r][c] = data->cloud_count;
			++data->cloud_areas[data->cloud_count-1];
			expand_cloud( r-1, c, data );
			expand_cloud( r, c-1, data );
			expand_cloud( r+1, c, data );
			expand_cloud( r, c+1, data );
	}
	else
		data->mirror[r][c] = -1;
}

bool exists( int r, int c, data_t * data ){
	return r >= data->init_row && r < data->fin_row && c >= 0 && c <= data->cols;
}

void print_clouds( std::vector<int> cloud_areas ){
	for( int i = 0; i < cloud_areas.size(); ++i )
		std::cout << i+1 << ": " << cloud_areas[i] << std::endl;
}

double ** read_image( data_t * data ){
	double ** image = ( double ** ) malloc( data->rows * sizeof( double * ) );
	for( int i = 0; i < data->rows; ++i )
		image[i] = ( double * ) malloc( data->cols * sizeof( double ) );

	for( int i = 0; i < data->rows; ++i ){
		for( int j = 0; i < data->cols; ++i )
			fscanf( data->file, "%lf", &image[i][j] );
	}

	return image;
}

int ** get_mirror( int begin, int end, int cols ){
	int ** mirror = ( int ** ) malloc( (end - begin) * sizeof( int * ) );
	for( int i = 0; i < end-begin; ++i )
		mirror[i] = ( int * ) calloc( cols, sizeof( int ) );

	return mirror;
}

int my_start( int rank, int D, int W ){
	return rank * D/W + std::min( rank, D%W );
}

void update_last_row( int to_mod, int mod, data_t * data ){
	int range = data->fin_row - data->init_row;
	for( int i = 0; i < data->cols; ++i )
		if( data->mirror[range-1][i] == to_mod )
			data->mirror[range-1][i] = mod;
}

void destroy_image( double ** image, int size ){
	for( int i = 0; i < size; ++i )
		free( image[i] );
	free( image );
}

void destroy_mirror( int ** mirror, int size ){
	for( int i = 0; i < size; ++i )
		free( mirror[i] );
	free( mirror );
}
