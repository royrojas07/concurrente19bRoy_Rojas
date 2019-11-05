#include <iostream>

int main(){
	#pragma omp parallel
	{
		#pragma omp critical(cout)
		std::cout << "Hello world from secondary thread" << std::endl;
	}
	
	return 0;
}
