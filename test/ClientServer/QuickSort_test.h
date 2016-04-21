#include "../../src/SerialImplementation/ClientServer/Structures/mergesort.h"
#include <cxxtest/TestSuite.h>


class MergeSortTest : public CxxTest::TestSuite{
	int *array;
	const int arraySize = 5;
	
public:
	void setUp()
	{
		array = new int[arraySize];
		array[0] = 20;
		array[1] = 40;
		array[2] = 4;
		array[3] = 1;
		array[4] = 9;
	}	
	
	void tearDown()
	{
		delete[] array;
	}
	

	void testMergeSort()
	{
		MergeSort(array, arraySize);
		for(int i=1; i<arraySize; ++i)
		{
			TS_ASSERT_LESS_THAN_EQUALS(array[i-1], array[i]);
		}
	}
};