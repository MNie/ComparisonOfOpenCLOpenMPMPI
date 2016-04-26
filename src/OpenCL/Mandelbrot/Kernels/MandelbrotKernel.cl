// OpenCL kernel
# pragma OPENCL EXTENSION cl_intel_printf :enable

#define SWRITING
#ifdef WRITING
#include "../../../../../../../usr/include/vector_types.h"
#define __kernel
#define __global
#endif

#define FLT_MIN 0.00000001f

typedef float2 Complex;

inline Complex Add(const Complex left, const Complex right) {
    return (Complex)(left.x+ right.x, left.y+ right.y);
}

inline Complex Multiply(const Complex left, const Complex right) {
    return (Complex)(left.x* right.x - left.y* right.y, left.x* right.y + left.y* right.x);
}

inline float Magnitude(const Complex value) {
    return sqrt(value.x*value.x + value.y*value.y);
}

inline int NormalizedIterations(const Complex value, const int n, const float bailout) {

    return n + (log(log(bailout)) - log(log(Magnitude(value)))) / log(2.0f);
}

inline int CalcuateSinglePoint(Complex point, int bailout)
{
    Complex startingPoint = (Complex)(0.0f,0.0f);
    for (int k = 0 ; k < bailout; ++k) {
        if (Magnitude(startingPoint) > 2.0f)
        {
            float returnValue = NormalizedIterations(startingPoint, k, bailout);
            return returnValue;
        }
        startingPoint = Multiply(startingPoint, startingPoint);
        startingPoint = Add(startingPoint, point);
    }
    return FLT_MIN;
}

typedef struct tag_parameters {
    int maxIterations;
    int height;
    int width;
    int totalLength;
} Parameters;


__kernel void MainKernel (__global Parameters *parameters,__global char *outputBuffer)
{
    int lid = get_local_id(0);
    int gid = get_global_id(0);
    int globalCount = get_global_size(0);
    int threadCount = get_local_size(0);
    Complex starting;
    int pixelsToCount = parameters->totalLength / globalCount;

    for (int j = 0; j < pixelsToCount; ++j) {

        starting.x = (-2.5f + 3.5f*((((float)((gid*pixelsToCount+j)%parameters->width)) / (float)parameters->width)));
        starting.y= -1.25f + 2.5f*(((((float)((gid*pixelsToCount+j)/parameters->width))  / (float)parameters->height)));

        outputBuffer[(gid*pixelsToCount) + j] = (char)(CalcuateSinglePoint(starting, 200));
    }
}