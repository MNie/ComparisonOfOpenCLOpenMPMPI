#include <limits.h>

#define ALLOC_ALIGN 64

void MergeSort(int array[], int length);
void MergeArray(int array[], int startLeftIndex, int endLeftIndex, int startRightIndex, int endRightIndex);

void MergeSort(int array[], int length)
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
	int leftArraySize = endLeftIndex - startLeftIndex + 1;
	int rightArraySize = endRightIndex - startRightIndex + 1;
	int* leftArray = (int*)_mm_malloc(sizeof(int) * leftArraySize, ALLOC_ALIGN);
	int* rightArray = (int*)_mm_malloc(sizeof(int) * rightArraySize, ALLOC_ALIGN);

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

	int k = startLeftIndex, i = 0, j = 0;
	for (; i < leftArraySize - 1 && j < rightArraySize - 1; ++k)
	{
		array[k] = leftArray[i] <= rightArray[j] ? leftArray[i++] : array[k] = rightArray[j++];
	}
	for (; i < leftArraySize - 1; ++k)
	{
		array[k] = leftArray[i++];
	}
	for (; j < rightArraySize - 1; ++k)
	{
		array[k] = rightArray[j++];
	}
	_mm_free(leftArray);
	_mm_free(rightArray);
}
