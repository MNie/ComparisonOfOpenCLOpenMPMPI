// OpenCL kernel
# pragma OPENCL EXTENSION cl_intel_printf :enable

#define SWRITING
#ifdef WRITING
#define __kernel
#define __global
#endif

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

typedef struct type_Pixel {
    char r, g, b;
    int drawed;
} Pixel;

typedef struct type_image {
    Pixel* Area;
} Image;

__kernel void ValueOfImage(__global Rectangle* rectangles, __global Parameters* parameters, __global Comparison* comparisonTable, __global Pixel* validImage, __global Pixel* imageToWriteOn)
{
    int globalId = get_global_id(0);
    int globalSize = get_global_size(0);
    int rectanglesPerThread = 1;
    float validsqrt = sqrt(255.0 * 255.0 * 3.0);
    float maxDiff = (float)parameters->width * (float)parameters->height * validsqrt;
    float diff = 0.0;
    if(parameters->population >= globalSize)
        rectanglesPerThread = parameters->population / (globalSize -1);
    int imageSize = parameters->width * parameters->height;
    int downLimit = globalId * rectanglesPerThread, upLimit = (globalId + 1) * rectanglesPerThread;
    for(int j=downLimit; j<upLimit; ++j)
    {
        if(j<parameters->population)
        {
            Rectangle rectangle;
            int height, width, baseY;
            for (int i = parameters->numberOfRectangles; i >= 0; --i) {
                rectangle = rectangles[j * parameters->numberOfRectangles + i];
                height = abs(rectangle.rightDown.y - rectangle.leftUp.y);
                width = abs(rectangle.rightDown.x - rectangle.leftUp.x);
                for (int y = height; y >= 0; --y) {
                    baseY = (rectangle.leftUp.y + y) * parameters->width;
                    for (int x = width; x >= 0; --x) {
                        int linearIndex = baseY + rectangle.leftUp.x + x;
                        imageToWriteOn[imageSize * globalId + linearIndex].r =
                                (imageToWriteOn[imageSize * globalId + linearIndex].r * imageToWriteOn[imageSize * globalId + linearIndex].drawed + rectangle.color.r) /
                                (imageToWriteOn[imageSize * globalId + linearIndex].drawed + 1);
                        imageToWriteOn[linearIndex].g =
                                (imageToWriteOn[imageSize * globalId + linearIndex].g * imageToWriteOn[imageSize * globalId + linearIndex].drawed + rectangle.color.g) /
                                (imageToWriteOn[imageSize * globalId + linearIndex].drawed + 1);
                        imageToWriteOn[linearIndex].b =
                                (imageToWriteOn[imageSize * globalId + linearIndex].b * imageToWriteOn[imageSize * globalId + linearIndex].drawed + rectangle.color.b) /
                                (imageToWriteOn[imageSize * globalId + linearIndex].drawed + 1);

                        ++imageToWriteOn[imageSize * globalId + linearIndex].drawed;
                    }
                }
            }
            Pixel firstPixel, secondPixel;
            for (int y = parameters->height - 1; y >= 0; --y) {
                for (int x = parameters->width - 1; x >= 0; --x) {
                    firstPixel = imageToWriteOn[imageSize * globalId + y * parameters->width + x];
                    secondPixel = validImage[y * parameters->width + x];
                    diff += sqrt((float)(
                            (firstPixel.r - secondPixel.r) * (firstPixel.r - secondPixel.r)
                            + (firstPixel.g - secondPixel.g) * (firstPixel.g - secondPixel.g)
                            + (firstPixel.b - secondPixel.b) * (firstPixel.b - secondPixel.b)));
                }
            }
            comparisonTable[j].value = (maxDiff - diff) / maxDiff;
            comparisonTable[j].index = j;
        }
    }
}
