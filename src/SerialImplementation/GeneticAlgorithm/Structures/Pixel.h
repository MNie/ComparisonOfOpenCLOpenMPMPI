#ifndef GENETICALGORITHM_PIXEL_H
#define GENETICALGORITHM_PIXEL_H

#include <bits/types.h>

struct Pixel {
    __uint8_t r, g, b;
    int drawed;

    Pixel(__uint8_t r, __uint8_t g, __uint8_t b);
};


#endif //GENETICALGORITHM_PIXEL_H
