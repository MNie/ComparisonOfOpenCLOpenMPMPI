#include "Comparison.h"

Comparison::Comparison(int i, double d) :
        index(i), value(d)
{
}

bool Comparison::CompareComparison (const Comparison& first, const Comparison& second)
{
    return first.value > second.value;
}