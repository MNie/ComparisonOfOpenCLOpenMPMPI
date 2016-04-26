// OpenCL kernel
# pragma OPENCL EXTENSION cl_intel_printf :enable

#define SWRITING
#ifdef WRITING
#include "../../../../../../../usr/include/vector_types.h"
#define __kernel
#define __global
#endif

#define ALLOC_ALIGN 64

inline void swap(__global int* left, __global int* right)
{
    int tmp = *right;
    *right = *left;
    *left = tmp;
}

inline int getDividePoint(__global int *array, int firstIndex, int lastIndex)
{
    return firstIndex + (lastIndex - firstIndex) / 2;
}

inline void MergeArray(__global int* array, int startLeftIndex, int endLeftIndex, int startRightIndex, int endRightIndex, __global int* localMemory)
{
    int leftArraySize = endLeftIndex - startLeftIndex + 1;
    int rightArraySize = endRightIndex - startRightIndex + 1;
    __global int* leftArray = localMemory;//(int*)malloc(sizeof(int) * leftArraySize);
    __global int* rightArray = localMemory + leftArraySize + 1;//(int*)malloc(sizeof(int) * rightArraySize);
    int k = startLeftIndex, i = 0, j = 0;
    for (int i = 0; i < leftArraySize - 1; ++i)
    {
        leftArray[i] = array[startLeftIndex + i];
    }
    leftArray[leftArraySize - 1] = INT_MAX;

    for (int i = 0; i < rightArraySize - 1; ++i)
    {
        rightArray[i] = array[startRightIndex + i];
    }
    rightArray[rightArraySize - 1] = INT_MAX;

	for (; i < leftArraySize - 1 && j < rightArraySize - 1; ++k)
	{
		array[k] = leftArray[i] <= rightArray[j] ? leftArray[i++] : rightArray[j++];
	}
	for (; i < leftArraySize - 1; ++k)
	{
		array[k] = leftArray[i++];
	}
	for (; j < rightArraySize - 1; ++k)
	{
		array[k] = rightArray[j++];
	}
}

inline void MergeSort(__global int* array, int length, __global int* localMemory)
{
    if (length < 2)
    {
        return;
    }

    int step = 1;
    while (step < length)
    {
        int startLeftIndex = 0;
        int startRightIndex = step;
        while (startRightIndex + step <= length)
        {
            MergeArray(array, startLeftIndex, startLeftIndex + step, startRightIndex, startRightIndex + step, localMemory);
            startLeftIndex = startRightIndex + step;
            startRightIndex = startLeftIndex + step;
        }
        if (startRightIndex < length)
        {
            MergeArray(array, startLeftIndex, startLeftIndex + step, startRightIndex, length, localMemory);
        }
        step *= 2;
    }
}

typedef struct tag_parameters {
    int arrayCounter;
    int arraySize;
} Parameters;


__kernel void MainKernel (__global int* localMemory, __global Parameters *parameters, __global int *outputBuffer)
{
    MergeSort(outputBuffer, parameters->arraySize, localMemory);
}
