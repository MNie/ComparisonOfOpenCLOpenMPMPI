#ifndef GENETICALGORITHM_LIGHTRAND_H
#define GENETICALGORITHM_LIGHTRAND_H
#include <stdlib.h>

class LightRand {
public:
    static unsigned long pa, npa, seed;
    static unsigned long x, y, z;
    static unsigned int Rand();
};

#endif //GENETICALGORITHM_LIGHTRAND_H
