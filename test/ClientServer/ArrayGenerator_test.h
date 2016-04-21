#include "../../src/SerialImplementation/ClientServer/Structures/arrayGenerator.h"
#include <cxxtest/TestSuite.h>


class ArrayGeneratorTest : public CxxTest::TestSuite{
	int *array;
	const int arraySize = 20;
	
public:
	void setUp()
	{
		array = new int[arraySize];
	}	
	
	void tearDown()
	{
		delete array;
	}
	

	void testArrayGenerator()
	{
		GenerateArray(array, arraySize);
		for(int i=0; i<arraySize; ++i)
		{
			TS_ASSERT_LESS_THAN_EQUALS(0, array[i]);
		}
	}
};