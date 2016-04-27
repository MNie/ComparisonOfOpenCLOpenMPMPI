#ifndef GENETICALGORITHM_GENETICALGORITHMKERNELPARAMETERS_H
#define GENETICALGORITHM_GENETICALGORITHMKERNELPARAMETERS_H
struct GeneticAlgorithmKernelParameters {
    int generation;
    int population;
    int width;
    int height;
    int elite;
    int numberOfRectangles;
};

typedef struct GeneticAlgorithmKernelParameters Parameter_kernel;
#endif //GENETICALGORITHM_GENETICALGORITHMKERNELPARAMETERS_H
