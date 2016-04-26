#include "RequestFetcher.h"
#include "arrayGenerator.h"

#define ALLOC_TRANSFER_ALIGN 4096

RequestFetcher::RequestFetcher(int requestSize) : requestSize(requestSize){

}

int* RequestFetcher::operator()(){
    auto array = (int*)_mm_malloc(requestSize * sizeof(int), ALLOC_TRANSFER_ALIGN);;
    GenerateArray(array, requestSize);
    return array;
}
