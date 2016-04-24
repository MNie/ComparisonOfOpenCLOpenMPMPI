#include "../GeneticAlgorithm.h"
#include <cxxtest/TestSuite.h>

class GeneticAlgorithmTest : public CxxTest::TestSuite, public GeneticAlgorithm
{
	Image first, second;
	int initialValue = 1, changedValue = 3;
public:
	void setUp()
	{
		this->width = 20;
		this->height = 20;
		first.Area = new Pixel[400];
		second.Area = new Pixel[400];
		for(int i=0; i<400; ++i)
		{
			first.Area[i].r = initialValue;
			first.Area[i].g = initialValue;
			first.Area[i].b = initialValue;
			
			second.Area[i].r = initialValue;
			second.Area[i].g = initialValue;
			second.Area[i].b = initialValue;	
		}
	}
	
	void tearDown()
	{
		delete [] first.Area;
		delete [] second.Area;
	}

	void testGettingNewRectangle()
	{
		Rectangle result = this->getNewRectangle();
		TS_ASSERT_LESS_THAN(result.rightDown.y, this->height);
		TS_ASSERT_LESS_THAN(result.rightDown.x, this->width);
		TS_ASSERT_LESS_THAN(result.leftUp.y, this->height);
		TS_ASSERT_LESS_THAN(result.leftUp.x, this->width);
		TS_ASSERT_LESS_THAN(result.color.r, 256);
		TS_ASSERT_LESS_THAN(result.color.g, 256);
		TS_ASSERT_LESS_THAN(result.color.b, 256);
	}
	
	void testClearingImage()
	{
		this->ClearImage(first, 20, 20)
		for(int i=0; i<400; ++i)
		{
			TS_ASSERT_EQUALS(0, first.Area[i].r);
			TS_ASSERT_EQUALS(0, first.Area[i].g);
			TS_ASSERT_EQUALS(0, first.Area[i].b);
			TS_ASSERT_EQUALS(0, first.Area[i].drawed);
		}
	}
	
	void testComparisonOfImages()
	{
		double result = this->compare(&first, &second);
		TS_ASSERT_EQUALS(1.0, result);
		
		//when we change values of a second array..
		for(int i=0; i<400; ++i)
		{
			second.Area[i].b = changedValue;	
		}
		
		double expectedResult = ((double)this->validsqrt - (double)(sqrt((changedValue - initialValue) * (changedValue - initialValue))))/(double)(this->validsqrt);
		TS_ASSERT_LESS_THAN(expectedResult - 0.01, result);
		TS_ASSERT_LESS_THAN(result, expectedResult + 0.01);
	}
};