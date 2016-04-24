#ifndef GENETICALGORITHM_RECTANGLE_H
#define GENETICALGORITHM_RECTANGLE_H

#include "Point.h"
#include "Color.h"

struct Rectangle
{
    Rectangle();
    Rectangle(Point leftUp, Point rightDown,  Color color);

    Point rightDown, leftUp;
    Color color;
};

#endif //GENETICALGORITHM_RECTANGLE_H
