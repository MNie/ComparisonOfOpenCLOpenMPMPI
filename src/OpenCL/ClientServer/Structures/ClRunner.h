#ifndef DIVIDEANDCONQUER_CLRUNNER_H
#define DIVIDEANDCONQUER_CLRUNNER_H

#define ALLOC_ALIGN 64


#include "../../../Libraries/OpenCL/cl_1.2.hpp"
#include "KernelParameters.h"
#include <cstdio>


class ClRunner {
private:
    cl::Event previousCommandEvent;
    cl::Event event;
public:
    cl::CommandQueue CommandQueue;
    cl::Context Context;

    cl::Buffer LocalMemoryBuffer;
    cl::Buffer OutBuffer;
    cl::Buffer InBuffer;
    KernelParameters Parameters;


    ClRunner(cl::Context Context, KernelParameters Parameters);
    virtual ~ClRunner();
    void operator()(cl::make_kernel<cl::Buffer &, cl::Buffer &, cl::Buffer &> Kernel, void (CL_CALLBACK *pfn_notify)(cl_event event, cl_int command_exec_status, void * user_data), void *pVoid);

    void SetParameters(KernelParameters parameters);

    void WriteTo(int *area, int count);
    void ReadTo(int *area);
    void ReadSingle(int *area);

    ClRunner();
};

struct RunnerArgs {
    void* PassedData;
    void* OutputData;
    int ElementsNumber;
    void (CL_CALLBACK *Callback)(cl_event event, cl_int command_exec_status, void * user_data);
    ClRunner* that;
    cl::CommandQueue* CommandQueue;
    cl::Buffer* OutputBuffer;
};

#endif //DIVIDEANDCONQUER_CLRUNNER_H
