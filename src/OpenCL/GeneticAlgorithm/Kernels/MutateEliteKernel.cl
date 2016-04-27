// OpenCL kernel
# pragma OPENCL EXTENSION cl_intel_printf :enable

#define SWRITING
#ifdef WRITING
#include "../../../../../../../usr/include/vector_types.h"
#define __kernel
#define __global
#endif

#define FLT_MIN 0.00000001f

typedef struct type_point
{
    unsigned int x, y;
} Point;

typedef struct type_color
{
    unsigned char r, g, b;
} Color;

typedef struct type_rect
{
    Point rightDown, leftUp;
    Color color;
} Rectangle;

typedef struct type_parameters
{
    int generation;
    int population;
    int width;
    int height;
    int elite;
    int numberOfRectangles;
} Parameters;

typedef struct type_compare {
    int index;
    float value;
} Comparison;

inline int rand(int firstSeed, int secondSeed, int globalID)
{
    int seed = firstSeed + globalID;
    int t = seed ^ (seed << 11);
    return secondSeed ^ (secondSeed >> 19) ^ (t ^ (t >> 8));
}

inline Rectangle getNewRectangle(__global Parameters* params, int globalId) {
    Rectangle rectangle;

    rectangle.rightDown.y = rand(params->numberOfRectangles, 1, globalId) % (params->height - 1) + 1;
    rectangle.leftUp.y = rand(params->numberOfRectangles, 2, globalId) % rectangle.rightDown.y;

    rectangle.rightDown.x = rand(params->numberOfRectangles, 3, globalId) % (params->width - 1) + 1;
    rectangle.leftUp.x = rand(params->numberOfRectangles, 4, globalId) % rectangle.rightDown.x;

    rectangle.color.r = rand(params->numberOfRectangles, 5, globalId) % 256;
    rectangle.color.g = rand(params->numberOfRectangles, 6, globalId) % 256;
    rectangle.color.b = rand(params->numberOfRectangles, 7, globalId) % 256;

    return rectangle;
}

__kernel void MutateElite(__global Rectangle* rectangles, __global Parameters* parameters, __global Comparison* comparisonResults)
{
    int globalId = get_global_id(0);
    int globalSize = get_global_size(0);
    int index = parameters->population - globalId;
    unsigned long i1 = rand(123, parameters->numberOfRectangles, globalId) % parameters->elite;
    int i2 = index % parameters->elite;
    __global Rectangle *mutateRectangle, *first, *second;

    int rectanglesPerThread = 1;
    if(parameters->population >= globalSize)
        rectanglesPerThread = parameters->population / (globalSize -1);
    int imageSize = parameters->width * parameters->height;
    int downLimit = globalId * rectanglesPerThread, upLimit = (globalId + 1) * rectanglesPerThread;

    for(int j=downLimit; j<upLimit; ++j)
    {
        if(j<parameters->population)
        {
            for (int i = 0; i < parameters->numberOfRectangles; ++i) {
                if (rand(12, parameters->numberOfRectangles, j) % 2 > 0) {
                    rectangles[(comparisonResults[j].index * parameters->numberOfRectangles) + i] = rand(2, parameters->numberOfRectangles, j) % 4 > 0 ?
                        rectangles[comparisonResults[i2].index * parameters->numberOfRectangles  + i] :
                        getNewRectangle(parameters, j);
                }
                else {
                    rectangles[(comparisonResults[j].index * parameters->numberOfRectangles) + i] = rand(1, parameters->numberOfRectangles, j) % 4 > 0 ?
                        rectangles[comparisonResults[i1].index * parameters->numberOfRectangles + i] :
                        getNewRectangle(parameters, j);
                }
            }
        }
    }
}