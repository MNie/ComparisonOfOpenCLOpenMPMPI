#ifndef GENETICALGORITHM_COMPARISION_H
#define GENETICALGORITHM_COMPARISION_H

struct Comparison
{
    Comparison(int i, double d);
    static bool CompareComparison (const Comparison& first, const Comparison& second);

    unsigned int index;
    double value;
};

#endif //GENETICALGORITHM_COMPARISION_H
