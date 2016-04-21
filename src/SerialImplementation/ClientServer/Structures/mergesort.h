#include <limits.h>
#include <mm_malloc.h>
#define ALLOC_ALIGN 64

void MergeSort(int array[], int length);
void MergeArray(int array[], int startLeftIndex, int endLeftIndex, int startRightIndex, int endRightIndex);

void MergeSort(int array[], int length)
{
	if (length < 2)
	{
		return;
	}

	auto step = 1;
	while (step < length)
	{
		auto startLeftIndex = 0;
		auto startRightIndex = step;
		while (startRightIndex + step <= length)
		{
			MergeArray(array, startLeftIndex, startLeftIndex + step, startRightIndex, startRightIndex + step);
			startLeftIndex = startRightIndex + step;
			startRightIndex = startLeftIndex + step;
		}
		if (startRightIndex < length)
		{
			MergeArray(array, startLeftIndex, startLeftIndex + step, startRightIndex, length);
		}
		step *= 2;
	}
}

void MergeArray(int array[], int startLeftIndex, int endLeftIndex, int startRightIndex, int endRightIndex)
{
	auto leftArraySize = endLeftIndex - startLeftIndex + 1;
	auto rightArraySize = endRightIndex - startRightIndex + 1;
	auto leftArray = (int*)_mm_malloc(sizeof(int) * leftArraySize, ALLOC_ALIGN);
	auto rightArray = (int*)_mm_malloc(sizeof(int) * rightArraySize, ALLOC_ALIGN);

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

	for (int k = startLeftIndex, i = 0, j = 0; k < endRightIndex; ++k)
	{
		array[k] = leftArray[i] <= rightArray[j] ? leftArray[i++] : rightArray[j++];
	}
	
	_mm_free(leftArray);
	_mm_free(rightArray);
}