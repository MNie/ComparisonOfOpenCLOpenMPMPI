#include <stdio.h>
#include <iostream>
#include <fstream>
#include "Structures/mergesort.h"
#include "Structures/arrayGenerator.h"
#include "../../Libraries/Timer.h"

#define ALLOC_ALIGN 64

int main(int argc, char** argv) {
    int arraySize, numberOfArrays;
    Timer mainTimer(Timer::Mode::Median);
    if (argc != 3) {
        printf("%d", argc);
        printf("Wrong number of arguments, correct number is: 1- sizeOfArray 2- numberOfArrays\n");
        return 0;
    }
    else {
        arraySize = atoi(argv[1]);
        numberOfArrays = atoi(argv[2]);
    }

	auto array = (int*)_mm_malloc(sizeof(int*) * numberOfArrays * arraySize, ALLOC_ALIGN);
    
    for(int arrayCounter = 0; arrayCounter < numberOfArrays; arrayCounter++)
    {
        GenerateArray(array + (arrayCounter * arraySize), arraySize);
		mainTimer.Start();
        MergeSort(array + (arrayCounter * arraySize), arraySize);
        mainTimer.Stop();
        std::cout << mainTimer.Get() << std::endl;
    }
    _mm_free(array);

    return 0;
}
