#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Structures/mergesort.h"
#include "Structures/arrayGenerator.h"
#include "../../Libraries/OpenCL/cl_1.2.hpp"
#include "Structures/KernelParameters.h"
#include "../../../../../../usr/include/stdlib.h"
#include "Structures/ClEnvironment.h"
#include "Structures/ClRunner.h"
#include "Structures/ProcessorParameters.h"
#include "Structures/RequestFetcher.h"
#include "Structures/RequestProcessor.h"
#include "../../Libraries/Timer.h"

#define ALLOC_ALIGN 64

void MakeCalculations(ClEnvironment &environment, int numberOfArrays, int arraySize, int numberOfRunners) ;

ClEnvironment PrepareEnvironment(int arrays, int size);

std::string GetFileContent(const char *string);

int main(int argc, char** argv) {
    Timer mainTimer(Timer::Mode::Single);
    Timer mainTimerWithoutEnv(Timer::Mode::Single);
    int arraySize, numberOfArrays, numberOfRunners;
    mainTimer.Start();

    if (argc != 4) {
        printf("%d", argc);
        printf("Wrong number of arguments, correct number is: 1- sizeOfArray 2- numberOfArrays\n");
        return 0;
    }
    else {
        arraySize = atoi(argv[1]);
        numberOfArrays = atoi(argv[2]);
        numberOfRunners = atoi(argv[3]);
    }

    auto context = cl::Context(CL_DEVICE_TYPE_ACCELERATOR);
    auto kernel = PrepareEnvironment(numberOfArrays, arraySize);
    mainTimerWithoutEnv.Start();
    MakeCalculations(kernel, numberOfArrays, arraySize, numberOfRunners);
    mainTimerWithoutEnv.Stop();
    mainTimer.Stop();
    printf(",%lu,%lu,", mainTimerWithoutEnv.Get(), mainTimer.Get());
    return 0;
}

ClEnvironment PrepareEnvironment(int arrays, int size) {
    cl_int error;
    auto context = cl::Context(CL_DEVICE_TYPE_ACCELERATOR);
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

    auto kernelSource = GetFileContent("Kernels/MergesortKernel.cl");

    char *buildOptions = "-cl-finite-math-only -cl-no-signed-zeros";
    auto program = cl::Program(context, kernelSource, false, &error);
    program.build(devices, buildOptions);
    if(error != CL_SUCCESS) {
        printf("Error at creating program : %d\n", error);
        printf("Build log %s \n", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]).c_str());
    }

    auto kernel = cl::make_kernel<cl::Buffer&, cl::Buffer&, cl::Buffer&> (program, "MainKernel", &error);
    if(error != CL_SUCCESS) printf("Error at getting kernel : %d\n", error);

    return ClEnvironment(kernel, context);
}

std::string GetFileContent(const char *string) {
    std::ifstream file(string, std::ios_base::in);

    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();

    return contents.str();
}

void MakeCalculations(ClEnvironment &environment, int numberOfArrays, int arraySize, int numberOfRunners) {
    auto kernel = environment.kernel;
    auto context = environment.context;
    cl_int error;

    KernelParameters parameters;
    parameters.arrayCounter = numberOfArrays;
    parameters.arraySize = arraySize;

    ProcessorParameters processorParameters(numberOfRunners, parameters, environment);

    RequestFetcher fetcher(arraySize);
    RequestProcessor processor(processorParameters, fetcher);
    processor.StartProcessing();
}
