#ifndef DIVIDEANDCONQUER_PROCESSORPARAMETERS_H
#define DIVIDEANDCONQUER_PROCESSORPARAMETERS_H


#include "KernelParameters.h"
#include "ClEnvironment.h"

class ProcessorParameters {
public:
    int numberOfRunners;
    KernelParameters kernelParameters;
    ClEnvironment environment;

    ProcessorParameters(int numberOfRunners, KernelParameters kernelParameters, ClEnvironment &environment);
};


#endif //DIVIDEANDCONQUER_PROCESSORPARAMETERS_H
