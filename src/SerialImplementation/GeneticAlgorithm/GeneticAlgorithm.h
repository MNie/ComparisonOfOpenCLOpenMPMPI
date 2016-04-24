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
#include <stdlib.h>
#include "Structures/Image.h"
#include "../../Libraries/Timer.h"

class GeneticAlgorithm
{
protected:
    int height, width, population, generation, elite;
    long sizeOfRectangleTable;
    png::image<png::rgb_pixel> inputImage;
    Image* outputImages;
    Rectangle* imagesRectangles = nullptr;
    Comparison *comparisonResults = nullptr;
    Image* nativeImage = nullptr;
    Timer mainTimer, scoreTimer, mutationTimer, generationTimer;
    
    static unsigned long validsqrt;

    double compare(Image *first, Image *second);

    Rectangle getNewRectangle();

    void mutation(Rectangle *source, Rectangle *first, Rectangle *second);

    void generateRectanglesForImages(Rectangle *rectangle);

    void drawRectanglesOnImage(Image &image, const Rectangle *rectangles);
    
    void generateRectangles();

    void mutateElite();

public:
    int GenerationsLeft;
    Image OutputImage;

    GeneticAlgorithm(int population, int generation, int elite, const char* fileName);

    Image * TransformPngToNativeImage(png::image<png::rgb_pixel> image);

    void Calculate();

    png::image<png::rgb_pixel> ConvertToPng(Image& image);

    void ClearImage(Image& image, int height, int width);

    double CalculateValueOfImage(const Rectangle* vector);

    void SortComparisions() const;

    ~GeneticAlgorithm();
};
#endif //GENETICALGORITHM_GENETICALGORITHM_H
