#include "../Structures/Color.h"
#include <cxxtest/TestSuite.h>

class ColorTest : public CxxTest::TestSuite{
public:
    void testEmptyConstructor() {
        Color *color = new Color();
        TS_ASSERT_EQUALS(0, color->r);
        TS_ASSERT_EQUALS(0, color->g);
        TS_ASSERT_EQUALS(0, color->b);
    }
};
