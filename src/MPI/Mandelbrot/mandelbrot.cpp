#define DATA 1
#define ENDGEN 2

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include "../../Libraries/Timer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Libraries/C++/stb_image_write.h"

#define PARAMETERS 0
#define RESULTS_DATA 1
#define STARTING 2
#define ALLOC_ALIGN 64
#define ALLOC_TRANSFER_ALIGN 4096
#define FLT_MIN 0.00000001f

unsigned char GrayValue(float n) {
    return (char)(n);
}
struct Complex {
    float x, y;
    Complex():y(0),x(0){};
    Complex(float r, float i):y(i),x(r){};
};

inline Complex Add(const Complex left, const Complex right) {
    return Complex(left.x+ right.x, left.y+ right.y);
}

inline Complex Multiply(const Complex left, const Complex right) {
    return Complex(left.x* right.x - left.y* right.y, left.x* right.y + left.y* right.x);
}

inline float Magnitude(const Complex value) {
    return sqrt(value.x*value.x + value.y*value.y);
}

inline int NormalizedIterations(const Complex value, const int n, const float bailout) {

    return n + (log(log(bailout)) - log(log(Magnitude(value)))) / log(2.0f);
}

inline int CalcuateSinglePoint(Complex point, int bailout)
{
    Complex startingPoint = Complex(0.0f,0.0f);
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

void Mandelbrot(char* output, unsigned int offset, unsigned int width, unsigned int height, unsigned int numberOfProcessPixels)
{
    Complex c;

    for (int i = 0; i < numberOfProcessPixels; ++i) {

        c.x = (-2.5f + 3.5f*(((i+offset) % width)/(float)width));
        c.y = (-1.25f + 2.5f*(((i+offset) / width)/(float)height));
        output[i] = (char)(CalcuateSinglePoint(c, 200));
    }
}

struct CoreStatus {
    int indexes[2];
    int currentlyCalculating;
};

int main(int argc, char** argv)
{
    Timer mainTimer(Timer::Mode::Single), mainTimerWithoutEnv(Timer::Mode::Single);
    mainTimer.Start();
    MPI_Init(&argc, &argv);
    mainTimerWithoutEnv.Start();
    int width, height;
    int commSize, commRank;
    int processNameLenght;
    int PACKAGE_AMOUNT;
    int singlePackageOffset = 1024;
    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
    MPI_Request request[commSize-1];
    MPI_Status statuses[commSize-1];
    if (commRank == 0)
    {
        if(argc != 2)
        {
            printf("%d", argc);
            printf("Wrong number of arguments, correct number is: 1- width\n");
            return 0;
        }
        width = atoi(argv[1]);
        height = width;

        singlePackageOffset = (width * height)/((commSize-1));//std::min((width * height)/(2*(commSize-1)), 1024);
        PACKAGE_AMOUNT = (width * height) / singlePackageOffset;

        int sendParameters[3];
        sendParameters[0] = width;
        sendParameters[1] = height;
        sendParameters[2] = singlePackageOffset;


        for (int i = 1; i < commSize; i++) {
            MPI_Isend(sendParameters, 3, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD, &request[i-1]);
        }
        MPI_Waitall(commSize-1, request, statuses);

    }
    else
    {
        int receiveParameters[3];
        MPI_Recv(receiveParameters, 3, MPI_INT, 0, PARAMETERS, MPI_COMM_WORLD, &status);
        width = receiveParameters[0];
        height = receiveParameters[1];
        singlePackageOffset = receiveParameters[2];
    }

    if (commRank == 0)
    {
        char *results = (char*)_mm_malloc(sizeof(char) * width * height, ALLOC_TRANSFER_ALIGN);

        MPI_Request request;
        MPI_Status status;
        int totalNumberOfIterations = PACKAGE_AMOUNT;
        auto indexProvider = [&](const int index) {
            return singlePackageOffset*index;
        };
        int outValue;

        CoreStatus* currentIndexes = (CoreStatus*)_mm_malloc((commSize-1)*sizeof(CoreStatus), ALLOC_ALIGN);
        for (int j = std::min(commSize-1, totalNumberOfIterations); j > 0 ; --j) {
            currentIndexes[j-1].indexes[0] = j-1;
            currentIndexes[j-1].currentlyCalculating = 0;
            MPI_Isend(currentIndexes[j-1].indexes, 1, MPI_INT, j, DATA, MPI_COMM_WORLD, &request);

        }
        for (int j = std::min(commSize-1, totalNumberOfIterations - (commSize-1)); j > 0 ; --j) {
            currentIndexes[j-1].indexes[1] = commSize-1+j-1;
            MPI_Isend(currentIndexes[j-1].indexes + 1, 1, MPI_INT, j, DATA, MPI_COMM_WORLD, &request);

        }
        double value;

        for (int i = PACKAGE_AMOUNT-1; i >= 2*(commSize-1); --i) {
            MPI_Probe(MPI_ANY_SOURCE, DATA, MPI_COMM_WORLD, &status);
            MPI_Recv(results + indexProvider(currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating]), singlePackageOffset, MPI_CHAR, status.MPI_SOURCE, DATA, MPI_COMM_WORLD, &status);

            currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating] = i;

            MPI_Isend(currentIndexes[status.MPI_SOURCE-1].indexes + currentIndexes[status.MPI_SOURCE-1].currentlyCalculating, 1, MPI_INT, status.MPI_SOURCE, DATA, MPI_COMM_WORLD, &request);

            currentIndexes[status.MPI_SOURCE-1].currentlyCalculating = (currentIndexes[status.MPI_SOURCE-1].currentlyCalculating +1)%2;
        }

        for (int i = std::min(2*(commSize-1), totalNumberOfIterations); i > 0; --i) {
            MPI_Probe(MPI_ANY_SOURCE, DATA, MPI_COMM_WORLD, &status);
            MPI_Recv(results + indexProvider(currentIndexes[status.MPI_SOURCE-1].indexes[currentIndexes[status.MPI_SOURCE-1].currentlyCalculating]), singlePackageOffset, MPI_CHAR, status.MPI_SOURCE, DATA, MPI_COMM_WORLD, &status);


            currentIndexes[status.MPI_SOURCE-1].currentlyCalculating = (currentIndexes[status.MPI_SOURCE-1].currentlyCalculating +1)%2;
        }

        for (int i = commSize-1; i > 0; --i) {
            MPI_Isend(nullptr, 0, MPI_INT, i, ENDGEN, MPI_COMM_WORLD,
                      &request);
        }
        _mm_free(currentIndexes);
        _mm_free(results);
    }
    else
    {
        char* results = (char*)_mm_malloc(2*singlePackageOffset, ALLOC_TRANSFER_ALIGN);
        int currentlyCalculating = 0;
        int inValue[2];
        int value[2];
        MPI_Status status[2];
        MPI_Request request[2];
        MPI_Recv(inValue, 1, MPI_INT,0, MPI_ANY_TAG, MPI_COMM_WORLD, status);
        while (status->MPI_TAG != ENDGEN){
            MPI_Irecv(inValue+((currentlyCalculating+1)%2), 1, MPI_INT,0, MPI_ANY_TAG, MPI_COMM_WORLD, request+((currentlyCalculating+1)%2));

            Mandelbrot(results+currentlyCalculating*singlePackageOffset, singlePackageOffset*inValue[currentlyCalculating], width, height, singlePackageOffset);

            MPI_Isend(results+currentlyCalculating*singlePackageOffset, singlePackageOffset ,MPI_CHAR ,0 ,DATA ,MPI_COMM_WORLD ,request + currentlyCalculating);
            currentlyCalculating = (currentlyCalculating+1)%2;

            MPI_Wait(request + currentlyCalculating, status);
        };
    }
    if (commRank == 0) {
        mainTimerWithoutEnv.Stop();
#ifndef __MIC__
        printf("MPI,%d,%d,%lu,", commSize - 1, width, mainTimerWithoutEnv.Get());
#endif
    }
    MPI_Finalize();
    if (commRank == 0) {
        mainTimer.Stop();
#ifndef __MIC__
        printf("%lu,", mainTimer.Get());
#endif
    }

    return 0;
}
