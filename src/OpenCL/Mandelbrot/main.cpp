#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../../Libraries/OpenCL/cl_1.2.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Libraries/C++/stb_image_write.h"
#include "../../Libraries/Timer.h"

#include "MandelbrotKernelParameters.h"

#define ALLOC_ALIGN 64
#define ALLOC_TRANSFER_ALIGN 4096

std::string GetFileContent(const char *string);

int main(int argc, char** argv) {
    int numberOfThreads;
    MandelbrotKernelParameters parameters;

    Timer mainTimer(Timer::Mode::Single), mainTimerWithoutSettingUpEnv(Timer::Mode::Single);
    mainTimer.Start();
    
    parameters.width = atoi(argv[1]);
    parameters.height = parameters.width;
    numberOfThreads = atoi(argv[2]);
    parameters.maxIterations = 200;

    
    cl_int error;
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_ACCELERATOR, &devices);

    auto context = cl::Context(CL_DEVICE_TYPE_ACCELERATOR);

    auto kernelSource = GetFileContent("Kernels/MandelbrotKernel.cl");

    char *buildOptions = "-cl-finite-math-only -cl-no-signed-zeros";
    auto program = cl::Program(context, kernelSource, false, &error);
    program.build(devices, buildOptions);
    if(error != CL_SUCCESS) {
        printf("Error at creating program : %d\n", error);
        printf("Build log %s \n", program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]));
    }

    auto kernel = cl::make_kernel<cl::Buffer&, cl::Buffer&> (program, "MainKernel", &error);
    if(error != CL_SUCCESS) printf("Error at getting kernel : %d\n", error);

    auto commandQueue = cl::CommandQueue(context, 0, &error);
    if(error != CL_SUCCESS) printf("Error at creating command queue : %d\n", error);

    mainTimerWithoutSettingUpEnv.Start();

    auto inBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(MandelbrotKernelParameters), NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating input buffer : %d\n", error);

    const auto sizeOfOutput = parameters.width * parameters.height * sizeof(char);
    auto outBuffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeOfOutput, NULL, &error);
    if(error != CL_SUCCESS) printf("Error at creating output buffer : %d\n", error);

    auto output = (char*)_mm_malloc(sizeOfOutput, ALLOC_ALIGN);
    if(!output) printf("Error when allocating memory on host\n");

    cl::Event event;

    auto totalLength = parameters.height*parameters.width;
    if( totalLength % numberOfThreads ){
        totalLength + (numberOfThreads - totalLength % numberOfThreads);
    }
    parameters.totalLength = totalLength;

    commandQueue.enqueueWriteBuffer(inBuffer, CL_FALSE, 0, sizeof(MandelbrotKernelParameters), &parameters, NULL, &event);

    auto kernelEvent = kernel(cl::EnqueueArgs(commandQueue, event, cl::NDRange(numberOfThreads), cl::NDRange(numberOfThreads == 16 ? 16 : 32)), inBuffer, outBuffer);
    std::vector< cl::Event > kernelEvents = {kernelEvent};
    commandQueue.enqueueReadBuffer(outBuffer, CL_FALSE, 0, sizeOfOutput, output, &kernelEvents, &event);
    commandQueue.finish();

    _mm_free(output);
    mainTimerWithoutSettingUpEnv.Stop();
    mainTimer.Stop();
    printf("OpenCL,%d,%d,%lu,%lu,", numberOfThreads, parameters.width, mainTimerWithoutSettingUpEnv.Get(),mainTimer.Get());
}

std::string GetFileContent(const char *string) {
    std::ifstream file(string, std::ios_base::in);

    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();

    return contents.str();
}
