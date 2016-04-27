#ifndef GENETICALGORITHM_GENETICALGORITHM_H
#define GENETICALGORITHM_GENETICALGORITHM_H
#ifdef __INTEL_COMPILER
#include <mpi.h>
#include "../../Libraries/Timer.h"
#else
#include <openmpi/mpi.h>
#endif
#include <functional>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <time.h>
#include <C++/png++-0.2.5/png.hpp>
#include <tgmath.h>
#include "Structures/Point.h"
#include "Structures/Comparison.h"
#include "Structures/Color.h"
#include "Structures/Rectangle.h"
#include "Structures/Pixel.h"
#include "Structures/LightRand.h"
#include "Image.h"

class GeneticAlgorithm
{
private:
    int height, width, population, generation, elite;
    long sizeOfRectangleTable;
#ifndef __MIC__
    png::image<png::rgb_pixel> inputImage;
#endif
    Timer mainTimer, scoreTimer, mutationTimer, generationTimer;
    Rectangle* imagesRectangles = nullptr;
    Comparison *comparisonResults = nullptr;
    Image* nativeImage = nullptr;

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

#ifndef __MIC__
    Image * TransformPngToNativeImage(png::image<png::rgb_pixel> image);

    png::image<png::rgb_pixel> ConvertToPng(Image& image);
#endif

    void Calculate();

    void ClearImage(Image& image, int height, int width);

    double CalculateValueOfImage(const Rectangle* vector);

    void SortComparisions() const;

    ~GeneticAlgorithm();

    int commSize;
    int lastError;
    int commRank;

    void CalculateValuesInParallel(int generationsLeft, int generation1);

    void DistributeCalculations(int starting, int ending, std::function<int(int)> adressProvider);

    MPI_Datatype mpi_pixel_type;

    void InitCustomTypes();

    MPI_Datatype mpi_point_type;
    MPI_Datatype mpi_color_type;
    MPI_Datatype mpi_rectangle_type;

    void ReceiveAndCalculate();

    bool IsGood(Rectangle rectangle);
};
#endif //GENETICALGORITHM_GENETICALGORITHM_H
