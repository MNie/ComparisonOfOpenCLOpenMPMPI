#include <cstdlib>

void GenerateArray(int array[], int size)
{
	for (int i = size - 1; i >= 0; --i)
	{
		array[i] = rand()%INT_MAX;
	}
}
