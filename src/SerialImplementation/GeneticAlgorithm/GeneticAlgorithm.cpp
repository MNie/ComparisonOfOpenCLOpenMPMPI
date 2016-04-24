#include "GeneticAlgorithm.h"
#include <mm_malloc.h>

#define MAX_NUMBER_OF_RECTANGLES 500
#define ALLOC_ALIGN 64
#define ALLOC_TRANSFER_ALIGN 4096


unsigned long GeneticAlgorithm::validsqrt = sqrtl(255 * 255 * 3);

void GeneticAlgorithm::mutateElite() {
    for (int i = this->population - 1; i >= this->elite; --i) {
        unsigned long i1 = rand() % this->elite;
        int i2 = i % this->elite;
        mutation(this->imagesRectangles + comparisonResults[i].index * MAX_NUMBER_OF_RECTANGLES,
                 this->imagesRectangles + comparisonResults[i2].index * MAX_NUMBER_OF_RECTANGLES,
                 this->imagesRectangles + comparisonResults[i1].index * MAX_NUMBER_OF_RECTANGLES);
    }
}

void GeneticAlgorithm::generateRectangles() {
    for (int j = this->population - 1; j >= 0; --j) {
        generateRectanglesForImages(this->imagesRectangles + j * MAX_NUMBER_OF_RECTANGLES);
    }
}

void GeneticAlgorithm::drawRectanglesOnImage(Image &image, const Rectangle *rectangles) {
    for (int i = MAX_NUMBER_OF_RECTANGLES; i >= 0; --i) {
        Rectangle rectangle = rectangles[i];
        auto height = abs(rectangle.rightDown.y - rectangle.leftUp.y);
        auto width = abs(rectangle.rightDown.x - rectangle.leftUp.x);
        for (int y = height - 1; y >= 0; --y) {
            int baseY = (rectangle.leftUp.y + y) * this->width;
            for (int x = width - 1; x >= 0; --x) {
                int linearIndex = baseY + rectangle.leftUp.x + x;
                image.Area[linearIndex].r =
                        (image.Area[linearIndex].r * image.Area[linearIndex].drawed + rectangle.color.r) /
                        (image.Area[linearIndex].drawed + 1);
                image.Area[linearIndex].g =
                        (image.Area[linearIndex].g * image.Area[linearIndex].drawed + rectangle.color.g) /
                        (image.Area[linearIndex].drawed + 1);
                image.Area[linearIndex].b =
                        (image.Area[linearIndex].b * image.Area[linearIndex].drawed + rectangle.color.b) /
                        (image.Area[linearIndex].drawed + 1);

                ++image.Area[linearIndex].drawed;
            }
        }
    }
}

void GeneticAlgorithm::generateRectanglesForImages(Rectangle *rectangle) {
    for (int i = 0; i < MAX_NUMBER_OF_RECTANGLES; ++i) {
        rectangle[i] = getNewRectangle();
    }
}

void GeneticAlgorithm::mutation(Rectangle *returnArrayOfRectangles, Rectangle *first, Rectangle *second) {
    for (int i = 0; i < MAX_NUMBER_OF_RECTANGLES; ++i) {
        if (rand() % 2 > 0) {
            returnArrayOfRectangles[i] = rand() % 4 > 0 ? first[i] : getNewRectangle();
        }
        else {
            returnArrayOfRectangles[i] = rand() % 4 > 0 ? second[i] : getNewRectangle();
        }
    }
}

Rectangle GeneticAlgorithm::getNewRectangle() {
    Rectangle rectangle;

    rectangle.rightDown.y = rand() % (this->height - 1) + 1;
    rectangle.leftUp.y = rand() % rectangle.rightDown.y;

    rectangle.rightDown.x = rand() % (this->width - 1) + 1;
    rectangle.leftUp.x = rand() % rectangle.rightDown.x;

    rectangle.color.r = rand() % 256;
    rectangle.color.g = rand() % 256;
    rectangle.color.b = rand() % 256;

    return rectangle;
}

double GeneticAlgorithm::compare(Image *first, Image *second) {
    double maxDiff = this->width * this->height * validsqrt;
    auto diff = 0.0;
    for (int y = this->height - 1; y >= 0; --y) {
        for (int x = this->width - 1; x >= 0; --x) {
            auto firstPixel = first->Area[y * width + x];
            auto secondPixel = second->Area[y * width + x];
            diff += sqrtl(
                    (firstPixel.r - secondPixel.r) * (firstPixel.r - secondPixel.r)
                    + (firstPixel.g - secondPixel.g) * (firstPixel.g - secondPixel.g)
                    + (firstPixel.b - secondPixel.b) * (firstPixel.b - secondPixel.b));
        }
    }
    return (maxDiff - diff) / maxDiff;
}

GeneticAlgorithm::GeneticAlgorithm(int population, int generation, int elite, const char *fileName) {
    mainTimer(Timer(Timer::Mode::Single)), generationTimer(Timer(Timer::Mode::Median)), scoreTimer(Timer(Timer::Mode::Median)), mutationTimer(Timer(Timer::Mode::Median)){
    mainTimer.Start();
    this->population = population;
    this->generation = generation;
    this->GenerationsLeft = 0;
    this->elite = elite;
    this->sizeOfRectangleTable = this->population * MAX_NUMBER_OF_RECTANGLES;
    png::image<png::rgb_pixel> loadImage(fileName);
    nativeImage = TransformPngToNativeImage(loadImage);
    inputImage = loadImage;
    this->height = inputImage.get_height();
    this->width = inputImage.get_width();
    this->OutputImage = Image();
    Image::InitImage(this->OutputImage, height, width);

    this->outputImages = (Image *) _mm_malloc(population * sizeof(Image), ALLOC_ALIGN);

    for (int i = 0; i < population; ++i) {
        Image::InitImage(this->outputImages[i], height, width);
    }
    this->imagesRectangles = (Rectangle *) _mm_malloc(this->sizeOfRectangleTable * sizeof(Rectangle), ALLOC_ALIGN);

    this->comparisonResults = (Comparison *) _mm_malloc(this->population * sizeof(Comparison), ALLOC_ALIGN);

}

void GeneticAlgorithm::Calculate() {
    generateRectangles();

    for (GenerationsLeft = generation; GenerationsLeft > 0; --GenerationsLeft) {
        generationTimer.Start();
        scoreTimer.Start();
        if (GenerationsLeft == generation) {
            for (int i = this->population - 1; i >= 0; --i) {
                this->comparisonResults[i] = Comparison(i, CalculateValueOfImage(
                        this->imagesRectangles + i * MAX_NUMBER_OF_RECTANGLES));
            }
        }
        else {
            for (int i = this->population - 1; i >= this->elite; --i) {
                this->comparisonResults[i] = Comparison(this->comparisonResults[i].index, CalculateValueOfImage(
                        this->imagesRectangles + this->comparisonResults[i].index * MAX_NUMBER_OF_RECTANGLES));
            }
        }
        scoreTimer.Stop();
        SortComparisions();
        auto theBest = this->comparisonResults;
        this->comparisonResults[this->population - 1].value << std::endl;

        mutationTimer.Start();
        mutateElite();
        mutationTimer.Stop();
        generationTimer.Start();
    }

    mainTimer.Stop();
    printf("%lu, %lu, %lu, %lu\n", mainTimer.Get(), generationTimer.Get(), scoreTimer.Get(), mutationTimer.Get());
    auto theBest = this->comparisonResults;
    ConvertToPng(outputImages[theBest->index]).write("result.png");
}

double GeneticAlgorithm::CalculateValueOfImage(const Rectangle *imageRectangles) {
    ClearImage(this->OutputImage, height, width);
    drawRectanglesOnImage(OutputImage, imageRectangles);
    return compare(this->nativeImage, &this->OutputImage);
}

void GeneticAlgorithm::SortComparisions() const {
    std::sort(comparisonResults, comparisonResults + population, Comparison::CompareComparison);
}

Image *GeneticAlgorithm::TransformPngToNativeImage(png::image<png::rgb_pixel> image) {
    const size_t imgHeight = image.get_height();
    const size_t imgWidth = image.get_width();

    Image *outArray = Image::CreateImage(imgHeight, imgWidth);

    for (int y = 0; y < imgHeight; ++y) {
        for (int x = 0; x < imgWidth; ++x) {
            outArray->Area[y * imgWidth + x] = Pixel(image[y][x].red, image[y][x].green, image[y][x].blue);
        }
    }

    return outArray;
}

png::image<png::rgb_pixel> GeneticAlgorithm::ConvertToPng(Image &image) {
    png::image<png::rgb_pixel> resultImage = png::image<png::rgb_pixel>(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            resultImage.set_pixel(x, y, png::rgb_pixel(image.Area[y * width + x].r, image.Area[y * width + x].g,
                                                       image.Area[y * width + x].b));
        }
    }
    return resultImage;
}

void GeneticAlgorithm::ClearImage(Image &image, int height, int width) {
    for (int i = width * height - 1; i >= 0; --i) {
        image.Area[i].r = 0;
        image.Area[i].g = 0;
        image.Area[i].b = 0;
        image.Area[i].drawed = 0;
    }
}

GeneticAlgorithm::~GeneticAlgorithm() {
    _mm_free(this->outputImages);
    _mm_free(this->comparisonResults);
    if (this->imagesRectangles != nullptr) {
        _mm_free(this->imagesRectangles);
    }
    if (this->comparisonResults != nullptr) {
        _mm_free(this->comparisonResults);
    }

    if (this->nativeImage != nullptr) {
        delete this->nativeImage;
    }
}