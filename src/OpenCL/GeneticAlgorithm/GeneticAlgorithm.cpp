#include "GeneticAlgorithm.h"

#define MAX_NUMBER_OF_RECTANGLES 500
#define ALLOC_ALIGN 64
#define ALLOC_TRANSFER_ALIGN 4096


std::string GetFileContent(const char *string) {
    std::ifstream file(string, std::ios_base::in);

    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();

    return contents.str();
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
    int numberOfRectangles = LightRand::Rand() % MAX_NUMBER_OF_RECTANGLES;
    for (int i = 0; i < numberOfRectangles; ++i) {
        rectangle[i] = getNewRectangle();
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

GeneticAlgorithm::GeneticAlgorithm(int population, int generation, int elite, const char *fileName, int numberOfThreads) :
    mainTimer(Timer(Timer::Mode::Single)), generationTimer(Timer(Timer::Mode::Median)), scoreTimer(Timer(Timer::Mode::Median)), mutationTimer(Timer(Timer::Mode::Median)), numberOfThreads(numberOfThreads), mainTimerWithoutEnv(Timer(Timer::Mode::Single)){
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
    GeneticAlgorithmKernelParameters params;
    params.population = this->population;
    params.generation = this->generation;
    params.width = this->width;
    params.height = this->height;
    params.numberOfRectangles = MAX_NUMBER_OF_RECTANGLES;
    params.elite = this->elite;
    std::vector<cl::Event> events;
    cl::Event paramsEvent, comparisonEvent, validImageEvent, imageToWriteOnEvent, rectanglesEvent, kernelEvent;
    cl_int error;
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

    auto context = cl::Context(CL_DEVICE_TYPE_ACCELERATOR);

    auto valueOfImageSource = GetFileContent("Kernels/ValueOfImageKernel.cl");
    auto mutateEliteSource = GetFileContent("Kernels/MutateEliteKernel.cl");
    
    char *buildOptions = "-cl-finite-math-only -cl-no-signed-zeros";
    auto mutateEliteProgram = cl::Program(context, mutateEliteSource, true, &error);
    auto valueOfImageProgram = cl::Program(context, valueOfImageSource, true, &error);
    mutateEliteProgram.build(devices, buildOptions);
    valueOfImageProgram.build(devices, buildOptions);
    
    if(error != CL_SUCCESS) {
        printf("Error at creating program : %d\n", error);
        printf("Build log %s \n", mutateEliteProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]));
    }
    auto valueOfImageKernel = cl::make_kernel<cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&, cl::Buffer&> (valueOfImageProgram, "ValueOfImage", &error);
    auto mutateEliteKernel = cl::make_kernel<cl::Buffer&, cl::Buffer&, cl::Buffer&> (mutateEliteProgram, "MutateElite", &error);
    if(error != CL_SUCCESS) printf("Error at getting kernel : %d\n", error);

    auto valueOfImageQueue = cl::CommandQueue(context, 0, &error);
    if(error != CL_SUCCESS) printf("Error at creating command queue : %d\n", error);

    mainTimerWithoutEnv.Start();
    auto paramsInBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(GeneticAlgorithmKernelParameters), NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating input buffer : %d\n", error);

    auto validImageSize = sizeof(Pixel) * params.width * params.height;
    auto validImageInOutBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, validImageSize, NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating input buffer : %d\n", error);

    auto validImageToWriteOnSize = sizeof(Pixel) * params.width * params.height * numberOfThreads;
    auto validImageToWriteOnInOutBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, validImageToWriteOnSize, NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating input buffer : %d\n", error);

    const auto sizeOfOutput = params.population * params.numberOfRectangles * sizeof(Rectangle);
    auto rectanglesInOutBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeOfOutput, NULL, &error);
    const auto sizeOfComparison = params.population * sizeof(Comparison);
    auto comparisonInOutBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeOfComparison, NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating output buffer : %d\n", error);
    Pixel* imagesToWriteOn = (Pixel*)_mm_malloc(validImageToWriteOnSize, ALLOC_ALIGN);

    generateRectangles();

    for (GenerationsLeft = generation; GenerationsLeft > 0; --GenerationsLeft) {

        generationTimer.Start();
        cl::Event::waitForEvents(events);
        scoreTimer.Start();
        if (GenerationsLeft == generation) {
            valueOfImageQueue.enqueueWriteBuffer(paramsInBuffer, CL_FALSE, 0, sizeof(GeneticAlgorithmKernelParameters), &params, NULL, &paramsEvent);
            events.push_back(paramsEvent);
            valueOfImageQueue.enqueueWriteBuffer(validImageInOutBuffer, CL_FALSE, 0, validImageSize, this->nativeImage->Area, NULL, &validImageEvent);
            events.push_back(validImageEvent);
            valueOfImageQueue.enqueueWriteBuffer(validImageToWriteOnInOutBuffer, CL_FALSE, 0, validImageToWriteOnSize, imagesToWriteOn, NULL, &imageToWriteOnEvent);
            events.push_back(imageToWriteOnEvent);

            valueOfImageQueue.enqueueWriteBuffer(rectanglesInOutBuffer, CL_FALSE, 0, sizeOfOutput, this->imagesRectangles, NULL, &rectanglesEvent);
            events.push_back(rectanglesEvent);

            valueOfImageKernel(cl::EnqueueArgs(valueOfImageQueue, events, cl::NDRange(numberOfThreads), cl::NDRange(numberOfThreads == 16 ? 16 : 32)),
                               rectanglesInOutBuffer, paramsInBuffer, comparisonInOutBuffer,
                               validImageInOutBuffer, validImageToWriteOnInOutBuffer);
	        valueOfImageQueue.finish();

            valueOfImageQueue.enqueueReadBuffer(comparisonInOutBuffer, CL_FALSE, 0, sizeOfComparison, this->comparisonResults, NULL, &comparisonEvent);
        }
        else {
            cl::Event::waitForEvents(events);

            valueOfImageQueue.enqueueWriteBuffer(rectanglesInOutBuffer, CL_FALSE, 0, sizeOfOutput, this->imagesRectangles, NULL, &rectanglesEvent);
            events.push_back(rectanglesEvent);

            valueOfImageKernel(cl::EnqueueArgs(valueOfImageQueue, events, cl::NDRange(numberOfThreads), cl::NDRange(numberOfThreads == 16 ? 16 : 32)),
                               rectanglesInOutBuffer, paramsInBuffer, comparisonInOutBuffer,
                               validImageInOutBuffer, validImageToWriteOnInOutBuffer);
            valueOfImageQueue.finish();

            valueOfImageQueue.enqueueReadBuffer(comparisonInOutBuffer, CL_FALSE, 0, sizeOfComparison, this->comparisonResults, NULL, &comparisonEvent);
            events.push_back(comparisonEvent);
        }
        cl::Event::waitForEvents(events);
        scoreTimer.Stop();
        SortComparisions();
#ifdef debug
        auto theBest = this->comparisonResults;
        std::ostringstream stringStream;
        stringStream << "results" << GenerationsLeft << ".png";
        std::string copyOfStr = stringStream.str();

        drawRectanglesOnImage(OutputImage, imagesRectangles + theBest[0].index * MAX_NUMBER_OF_RECTANGLES);

        ConvertToPng(OutputImage).write(copyOfStr.c_str());
#endif
        mutationTimer.Start();
        valueOfImageQueue.enqueueWriteBuffer(rectanglesInOutBuffer, CL_FALSE, 0, sizeOfOutput, this->imagesRectangles, NULL, &rectanglesEvent);
        events.push_back(rectanglesEvent);

        mutateEliteKernel(cl::EnqueueArgs(valueOfImageQueue, events, cl::NDRange(numberOfThreads), cl::NDRange(numberOfThreads == 16 ? 16 : 32)),
                          rectanglesInOutBuffer, paramsInBuffer, comparisonInOutBuffer);
        valueOfImageQueue.finish();

        valueOfImageQueue.enqueueReadBuffer(rectanglesInOutBuffer, CL_FALSE, 0, sizeOfOutput, this->imagesRectangles, NULL, &rectanglesEvent);
        events.push_back(rectanglesEvent);
        mutationTimer.Stop();
        generationTimer.Stop();
    }
    cl::Event::waitForEvents(events);

    _mm_free(imagesToWriteOn);
    mainTimerWithoutEnv.Stop();
    mainTimer.Stop();
    printf("OpenCL,%d,%d,%d,%lu,%lu,%lu,%lu,%lu,", numberOfThreads, width, population , mainTimer.Get(), generationTimer.Get(), scoreTimer.Get(), mutationTimer.Get(), mainTimerWithoutEnv.Get());
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

GeneticAlgorithm::~GeneticAlgorithm() {

    for (int i = 0; i < population; ++i) {
        _mm_free(this->outputImages[i].Area);
    }
    if(this->outputImages != nullptr)
        _mm_free(this->outputImages);
    if (this->imagesRectangles != nullptr)
        _mm_free(this->imagesRectangles);

    if (this->comparisonResults != nullptr)
        _mm_free(this->comparisonResults);

    if (this->nativeImage->Area != nullptr)
        _mm_free(this->nativeImage->Area);

    if (this->nativeImage != nullptr)
        _mm_free(this->nativeImage);
}
