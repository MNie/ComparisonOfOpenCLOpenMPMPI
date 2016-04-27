#include "Image.h"

#define ALLOC_ALLIGN 64

Image *Image::CreateImage(int height, int width) {
    Image* image = (Image*)_mm_malloc(sizeof(image), ALLOC_ALLIGN);
    image->Area = (Pixel*)_mm_malloc(height*width*sizeof(Pixel), ALLOC_ALLIGN);
    return image;
}

void Image::InitImage(Image &image, int height, int width) {
    image.Area = (Pixel*)_mm_malloc(height*width*sizeof(Pixel), ALLOC_ALLIGN);
}
