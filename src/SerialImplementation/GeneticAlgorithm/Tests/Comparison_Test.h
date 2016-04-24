#include "../Structures/Comparison.h"
#include <cxxtest/TestSuite.h>

class ComparisonTest : CxxTest::TestSuite
{
    Comparison *first, *second;
public:
    void setUp()
    {
        first = new Comparison(1, 6.66);
        second = new Comparison(2, 4.44);
    }

    void tearDown()
    {
        delete first;
        delete second;
    }

    void testComparisonMethod()
    {
        TS_ASSERT_EQUALS(true, Comparison::CompareComparison(first, second));
    }
};