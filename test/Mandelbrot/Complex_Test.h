#include "../../src/SerialImplementation/Mandelbrot/Structures/Complex.h"
#include <cxxtest/TestSuite.h>


class ComplexTest : public CxxTest::TestSuite, public Complex{
	Complex *second;
	
public:
	void setUp()
	{
		this->Real = 0;
		this->Imaginary = 1;
		second = new Complex(2, 3);
	}	
	
	void tearDown()
	{
		delete second;
	}
	
	void testValidInitializationData()
	{
		TS_ASSERT_EQUALS(0, this->Real);
		TS_ASSERT_EQUALS(1, this->Imaginary);
		TS_ASSERT_EQUALS(2, second->Real);
		TS_ASSERT_EQUALS(3, second->Imaginary);
	}

	//private methods
	void testMultiply()
	{
		Complex result = this->multiply(*second);
		TS_ASSERT_EQUALS(-3.0, result.Real);
		TS_ASSERT_EQUALS(2.0, result.Imaginary);
	}
	
	void testLength()
	{
		float result = this->length();
		TS_ASSERT_EQUALS(1, result);
	}
	
	void testAdd()
	{
		Complex result = this->add(*second);
		TS_ASSERT_EQUALS(2.0, result.Real);
		TS_ASSERT_EQUALS(4.0, result.Imaginary);
	}
	
	void testBoundedOrbit()
	{
		float result = this->BoundedOrbit(second, 3.0f, 5);
		TS_ASSERT_LESS_THAN(-0.72f, result);
		TS_ASSERT_LESS_THAN(result, -0.71f);
	}
};