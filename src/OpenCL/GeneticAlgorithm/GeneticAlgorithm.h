#ifndef GENETICALGORITHM_GENETICALGORITHM_H
#define GENETICALGORITHM_GENETICALGORITHM_H
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <time.h>
#include "../../Libraries/C++/png++-0.2.5/png.hpp"
#include <tgmath.h>
#include "Structures/Point.h"
#include "Structures/Comparison.h"
#include "Structures/Color.h"
#include "Structures/Rectangle.h"
#include "Structures/Pixel.h"
#include "Structures/LightRand.h"
#include "Structures/Image.h"
#include "Structures/GeneticAlgorithmKernelParameters.h"
#include "../../Libraries/OpenCL/cl_1.2.hpp"
#include "../../Libraries/Timer.h"

class GeneticAlgorithm
{
private:
    int height, width, population, generation, elite, numberOfThreads;
    long sizeOfRectangleTable;
    png::image<png::rgb_pixel> inputImage;
    Image* outputImages = nullptr;
    Rectangle* imagesRectangles = nullptr;
    Comparison *comparisonResults = nullptr;
    Image* nativeImage = nullptr;
    Timer mainTimer, scoreTimer, mutationTimer, generationTimer, mainTimerWithoutEnv;

    Rectangle getNewRectangle();

    void generateRectanglesForImages(Rectangle *rectangle);

    void drawRectanglesOnImage(Image &image, const Rectangle *rectangles);

    void generateRectangles();

public:
    int GenerationsLeft;
    Image OutputImage;

    GeneticAlgorithm(int population, int generation, int elite, const char* fileName, int numberOfThreads);

    Image * TransformPngToNativeImage(png::image<png::rgb_pixel> image);

    void Calculate();

    png::image<png::rgb_pixel> ConvertToPng(Image& image);

    void SortComparisions() const;

    ~GeneticAlgorithm();
};
#endif //GENETICALGORITHM_GENETICALGORITHM_H
