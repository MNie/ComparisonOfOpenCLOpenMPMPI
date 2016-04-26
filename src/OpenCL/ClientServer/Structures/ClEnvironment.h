#ifndef DIVIDEANDCONQUER_CLENVIRONMENT_H
#define DIVIDEANDCONQUER_CLENVIRONMENT_H


#include "../../../Libraries/OpenCL/cl_1.2.hpp"

struct ClEnvironment {
    ClEnvironment(cl::make_kernel<cl::Buffer &, cl::Buffer &, cl::Buffer &> kernel, const cl::Context &context) : kernel(
            kernel), context(context) { }

    cl::make_kernel<cl::Buffer&, cl::Buffer &, cl::Buffer &> kernel;
    cl::Context context;
};


#endif //DIVIDEANDCONQUER_CLENVIRONMENT_H
