#include "../../src/SerialImplementation/Mandelbrot/Structures/mandelbrot.h"
#include <cxxtest/TestSuite.h>

class MandelbrotTest : public CxxTest::TestSuite
{
public:
	const int width = 1000, height = 500, offset = 0;
	char *table;

	void setUp()
	{
		table = new char[width*height];
	}	
	
	void tearDown()
	{
		delete[] table;
	}
	
	void testMandelbrot()
	{
		Mandelbrot(table, offset, width, height);
		bool result;
		for(int i= width * height -1; i >= 0; --i)
		{
			result |= table[i] != 0;
		}
		TS_ASSERT_EQUALS(true, result);
	}
};