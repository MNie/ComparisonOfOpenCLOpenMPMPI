#include "GeneticAlgorithm.h"
#define MAX_NUMBER_OF_RECTANGLES 500
#define ALLOC_ALIGN 64
#define ALLOC_TRANSFER_ALIGN 4096
#define DATA 1
#define ENDGEN 2

unsigned long GeneticAlgorithm::validsqrt = sqrtl(255 * 255 * 3);

void GeneticAlgorithm::mutateElite() {
    for (int i = this->population - 1; i >= this->elite; --i) {
        unsigned long i1 = LightRand::Rand() % this->elite;
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
    for (int i = MAX_NUMBER_OF_RECTANGLES-1; i >= 0; --i) {

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
    int numberOfRectangles = MAX_NUMBER_OF_RECTANGLES;
    for (int i = 0; i < numberOfRectangles; ++i) {
        rectangle[i] = getNewRectangle();
    }
}

void GeneticAlgorithm::mutation(Rectangle *returnArrayOfRectangles, Rectangle *first, Rectangle *second) {
    for (int i = 0; i < MAX_NUMBER_OF_RECTANGLES; ++i) {
        if (LightRand::Rand() % 2 > 0) {
            returnArrayOfRectangles[i] = LightRand::Rand() % 4 > 0 ? first[i] : getNewRectangle();
        }
        else {
            returnArrayOfRectangles[i] = LightRand::Rand() % 4 > 0 ? second[i] : getNewRectangle();
        }
    }
}

Rectangle GeneticAlgorithm::getNewRectangle() {
    Rectangle rectangle;

    rectangle.rightDown.y = LightRand::Rand() % (this->height - 1) + 1;
    rectangle.leftUp.y = LightRand::Rand() % rectangle.rightDown.y;

    rectangle.rightDown.x = LightRand::Rand() % (this->width - 1) + 1;
    rectangle.leftUp.x = LightRand::Rand() % rectangle.rightDown.x;

    rectangle.color.r = LightRand::Rand() % 256;
    rectangle.color.g = LightRand::Rand() % 256;
    rectangle.color.b = LightRand::Rand() % 256;

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

GeneticAlgorithm::GeneticAlgorithm(int population, int generation, int elite, const char *fileName) :
 mainTimer(Timer(Timer::Mode::Single)), generationTimer(Timer(Timer::Mode::Median)), scoreTimer(Timer(Timer::Mode::Median)), mutationTimer(Timer(Timer::Mode::Median)){
    mainTimer.Start();
    lastError = MPI_Comm_size(MPI_COMM_WORLD, &this->commSize);
    lastError = MPI_Comm_rank(MPI_COMM_WORLD, &this->commRank);

    this->population = population;
    this->generation = generation;
    this->GenerationsLeft = 0;
    this->elite = elite;
    this->sizeOfRectangleTable = this->population * MAX_NUMBER_OF_RECTANGLES;


    InitCustomTypes();

    if(commRank == 0) {
#ifndef __MIC__
        png::image<png::rgb_pixel> loadImage(fileName);
        nativeImage = TransformPngToNativeImage(loadImage);
        inputImage = loadImage;
        this->height = inputImage.get_height();
        this->width = inputImage.get_width();
#endif
    }

    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(commRank != 0) {
        nativeImage = Image::CreateImage(width,height);

    }
    MPI_Bcast(nativeImage->Area, width*height, mpi_pixel_type, 0, MPI_COMM_WORLD);
    this->OutputImage = Image();
    Image::InitImage(this->OutputImage, height, width);

    if(commRank == 0) {
        this->imagesRectangles = (Rectangle *) _mm_malloc(this->sizeOfRectangleTable * sizeof(Rectangle), ALLOC_TRANSFER_ALIGN);

        this->comparisonResults = (Comparison *) _mm_malloc(this->population * sizeof(Comparison), ALLOC_ALIGN);
    }
    else {
        this->imagesRectangles = (Rectangle *) _mm_malloc(2*MAX_NUMBER_OF_RECTANGLES * sizeof(Rectangle), ALLOC_ALIGN);

        this->comparisonResults = nullptr;
    }
}

GeneticAlgorithm::~GeneticAlgorithm() {
    if (this->imagesRectangles != nullptr) {
        _mm_free(this->imagesRectangles);
    }

    if(commRank == 0) {
        if (this->comparisonResults != nullptr) {
            _mm_free(this->comparisonResults);
        }
    }

    if (this->nativeImage != nullptr) {
        _mm_free(this->nativeImage);
    }
}

void GeneticAlgorithm::InitCustomTypes() {
    const int nitemsPixel=4;
    int          blocklengthsPixel[4] = {1,1,1,1};
    MPI_Datatype typesPixel[4] = {MPI_CHAR, MPI_CHAR,MPI_CHAR, MPI_INT};
    MPI_Aint     offsetsPixel[4];

    offsetsPixel[0] = offsetof(Pixel, r);
    offsetsPixel[1] = offsetof(Pixel, g);
    offsetsPixel[2] = offsetof(Pixel, b);
    offsetsPixel[3] = offsetof(Pixel, drawed);

    MPI_Type_create_struct(nitemsPixel, blocklengthsPixel, offsetsPixel, typesPixel, &mpi_pixel_type);
    MPI_Type_commit(&mpi_pixel_type);

    const int nitemsPoint=2;
    int          blocklengthsPoint[2] = {1, 1};
    MPI_Datatype typesPoint[2] = {MPI_UINT32_T, MPI_UINT32_T};
    MPI_Aint     offsetsPoint[2];

    offsetsPoint[0] = offsetof(Point, x);
    offsetsPoint[1] = offsetof(Point, y);
    MPI_Type_create_struct(nitemsPoint, blocklengthsPoint, offsetsPoint, typesPoint, &mpi_point_type);
    MPI_Type_commit(&mpi_point_type);

    const int nitemsColor=3;
    int          blocklengthsColor[3] = {1,1,1};
    MPI_Datatype typesColor[3] = {MPI_CHAR, MPI_CHAR,MPI_CHAR};
    MPI_Aint     offsetsColor[3];

    offsetsColor[0] = offsetof(Color, r);
    offsetsColor[1] = offsetof(Color, g);
    offsetsColor[2] = offsetof(Color, b);

    MPI_Type_create_struct(nitemsColor, blocklengthsColor, offsetsColor, typesColor, &mpi_color_type);
    MPI_Type_commit(&mpi_color_type);

    const int nitemsRectangle=3;
    int          blocklengthsRectangle[3] = {1,1,1};
    MPI_Datatype typesRectangle[3] = {mpi_point_type, mpi_point_type, mpi_color_type};
    MPI_Aint     offsetsRectangle[3];

    offsetsRectangle[0] = offsetof(Rectangle, rightDown);
    offsetsRectangle[1] = offsetof(Rectangle, leftUp);
    offsetsRectangle[2] = offsetof(Rectangle, color);

    MPI_Type_create_struct(nitemsRectangle, blocklengthsRectangle, offsetsRectangle, typesRectangle, &mpi_rectangle_type);
    MPI_Type_commit(&mpi_rectangle_type);
}

void GeneticAlgorithm::Calculate() {
    if(commRank == 0) {
        generateRectangles();
    }

    for (GenerationsLeft = generation; GenerationsLeft > 0; --GenerationsLeft) {
        if(commRank == 0) {
            generationTimer.Start();

            scoreTimer.Start();
            CalculateValuesInParallel(GenerationsLeft, generation);
            scoreTimer.Stop();
            SortComparisions();
            auto theBest = this->comparisonResults;
#ifdef debug
            std::ostringstream stringStream;
            stringStream << "results" << GenerationsLeft << ".png";
            std::string copyOfStr = stringStream.str();

            drawRectanglesOnImage(OutputImage, imagesRectangles + theBest[0].index * MAX_NUMBER_OF_RECTANGLES);

            ConvertToPng(OutputImage).write(copyOfStr.c_str());
#endif
            mutationTimer.Start();
            mutateElite();
            mutationTimer.Stop();
            generationTimer.Stop();
        }
        else {
            ReceiveAndCalculate();
        }
    }
    if(commRank == 0) {
#ifndef __MIC__
        mainTimer.Stop();
        printf("MPI,%d,%d,%d,%lu,%lu,%lu,%lu,", commSize - 1, width, population , mainTimer.Get(), generationTimer.Get(), scoreTimer.Get(), mutationTimer.Get());
#endif
    }
}

void GeneticAlgorithm::CalculateValuesInParallel(int generationsLeft, int generation1) {
    if (generationsLeft == generation1) {
        DistributeCalculations(population - 1, 0, [](int index) {
            return index;
        });
    }
    else {
        DistributeCalculations(population - 1, elite, [this](int index) {
            return this->comparisonResults[index].index;
        });
    }
}

struct CoreStatus {
    int indexes[2];
    int currentlyCalculating;
};

void GeneticAlgorithm::DistributeCalculations(int starting, int ending, std::function<int(int)> indexProvider) {
    MPI_Request request;
    MPI_Status status;
    int totalNumberOfIterations = starting - ending  + 1;

    CoreStatus* currentIndexes = (CoreStatus*)_mm_malloc((commSize-1)*sizeof(CoreStatus), ALLOC_ALIGN);
    for (int j = std::min(commSize-1, totalNumberOfIterations); j > 0 ; --j) {
        currentIndexes[j-1].indexes[0] = indexProvider(ending + j-1);
        currentIndexes[j-1].currentlyCalculating = 0;
        MPI_Isend(imagesRectangles + indexProvider(ending + j-1) * MAX_NUMBER_OF_RECTANGLES, MAX_NUMBER_OF_RECTANGLES, mpi_rectangle_type, j, DATA, MPI_COMM_WORLD, &request);

    }
    for (int j = std::min(commSize-1, totalNumberOfIterations - (commSize-1)); j > 0 ; --j) {
        currentIndexes[j-1].indexes[1] = indexProvider(ending + commSize-1+j-1);
        MPI_Isend(imagesRectangles + indexProvider(ending + commSize-1+j-1) * MAX_NUMBER_OF_RECTANGLES, MAX_NUMBER_OF_RECTANGLES, mpi_rectangle_type, j, DATA, MPI_COMM_WORLD, &request);

    }
    double value;

    for (int i = starting; i >= ending+2*(commSize-1); --i) {
        MPI_Recv(&value, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        comparisonResults[currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating]] =
                Comparison(indexProvider(currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating]),
                    value);

        MPI_Isend(imagesRectangles + indexProvider(i) * MAX_NUMBER_OF_RECTANGLES, MAX_NUMBER_OF_RECTANGLES, mpi_rectangle_type, status.MPI_SOURCE, DATA, MPI_COMM_WORLD, &request);

        currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating] = indexProvider(i);// ending+2*commSize+i; //i;?
        currentIndexes[status.MPI_SOURCE-1].currentlyCalculating = (currentIndexes[status.MPI_SOURCE-1].currentlyCalculating +1)%2;
    }

    for (int i = std::min(2*(commSize-1), totalNumberOfIterations); i > 0; --i) {
        MPI_Recv(&value, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        comparisonResults[currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating]] =
                Comparison(indexProvider(currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating]),
                           value);

        currentIndexes[status.MPI_SOURCE-1].currentlyCalculating = (currentIndexes[status.MPI_SOURCE-1].currentlyCalculating +1)%2;
    }

    for (int i = commSize-1; i > 0; --i) {
        MPI_Isend(nullptr, 0, MPI_INT, i, ENDGEN, MPI_COMM_WORLD,
                  &request);
    }
    _mm_free(currentIndexes);
}

void GeneticAlgorithm::ReceiveAndCalculate() {
    int currentlyCalculating = 0;
    double value[2];
    MPI_Status status[2];
    MPI_Request request[2];
    request[0] = 0;
    request[1] = 0;
    MPI_Recv(imagesRectangles, MAX_NUMBER_OF_RECTANGLES, mpi_rectangle_type,0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
    while (status->MPI_TAG != ENDGEN){
        MPI_Irecv(imagesRectangles+((currentlyCalculating+1)%2)*MAX_NUMBER_OF_RECTANGLES, MAX_NUMBER_OF_RECTANGLES, mpi_rectangle_type,0, MPI_ANY_TAG, MPI_COMM_WORLD, request+((currentlyCalculating+1)%2));

        value[currentlyCalculating] = CalculateValueOfImage(imagesRectangles+currentlyCalculating*MAX_NUMBER_OF_RECTANGLES);

        MPI_Isend(value+currentlyCalculating,1,MPI_DOUBLE,0,DATA,MPI_COMM_WORLD,request + currentlyCalculating);
        currentlyCalculating = (currentlyCalculating+1)%2;

        MPI_Wait(request + currentlyCalculating, status);
    };
    if(request[(currentlyCalculating+1)%2] != 0)
        MPI_Wait(request + (currentlyCalculating+1)%2, status);
}

double GeneticAlgorithm::CalculateValueOfImage(const Rectangle *imageRectangles) {
    ClearImage(this->OutputImage, height, width);
    drawRectanglesOnImage(OutputImage, imageRectangles);
    return compare(this->nativeImage, &this->OutputImage);
}

void GeneticAlgorithm::SortComparisions() const {
    std::sort(comparisonResults, comparisonResults + population, Comparison::CompareComparison);
}

#ifndef __MIC__
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
#endif

#ifndef __MIC__
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
#endif

void GeneticAlgorithm::ClearImage(Image &image, int height, int width) {
    for (int i = width * height - 1; i >= 0; --i) {
        image.Area[i].r = 0;
        image.Area[i].g = 0;
        image.Area[i].b = 0;
        image.Area[i].drawed = 0;
    }
}

bool GeneticAlgorithm::IsGood(Rectangle rectangle) {
    bool result = rectangle.rightDown.x > rectangle.leftUp.x &&
            rectangle.rightDown.y > rectangle.leftUp.y &&
            rectangle.rightDown.x < width &&
    rectangle.rightDown.y < height;

    return result;
}
