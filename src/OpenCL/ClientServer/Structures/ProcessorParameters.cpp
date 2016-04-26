#include "ProcessorParameters.h"
#include "ClEnvironment.h"

ProcessorParameters::ProcessorParameters(int numberOfRunners, KernelParameters kernelParameters, ClEnvironment &environment)
        : numberOfRunners{numberOfRunners},
          kernelParameters(kernelParameters),
          environment(environment) {

}
