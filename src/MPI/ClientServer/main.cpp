#include <mpi.h>
#include <stdio.h>
#include <fstream>
#include <mm_malloc.h>
#include <cstdlib>
#include <omp.h>
#include "Structures/mergesort.h"
#include "Structures/ArrayGenerator.h"
#include "../../Libraries/Timer.h"
#include <queue>
#include <mutex>

#define ALLOC_ALIGN_TRANSFER 4096
#define PARAMETERS 0
#define RESULTS_DATA 1
#define STARTING 2
#define STATUS 3
#define FINISH 4

int main(int argc, char **argv) {
    int arraySize, numberOfArrays, me, numberOfProcesses;
    Timer mainTimer(Timer::Mode::Single);
    Timer sendTimer(Timer::Mode::Single);
    Timer recvTimer(Timer::Mode::Single);
    Timer medianTimer(Timer::Mode::Median);
    Timer mainTimerWithoutEnv(Timer::Mode::Single);
    mainTimer.Start();
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    mainTimerWithoutEnv.Start();
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Status status;
    int *array;
    if (me == 0) {
        if (argc != 3) {
            printf("%d", argc);
            printf("Wrong number of arguments, correct number is: 1- sizeOfArray 2- numberOfArrays\n");
            return 0;
        }
        arraySize = atoi(argv[1]);
        numberOfArrays = atoi(argv[2]);

        int sendParameters[2];
        sendParameters[0] = arraySize;
        sendParameters[1] = numberOfArrays;

        for (int i = 1; i < numberOfProcesses; ++i) {
            MPI_Send(sendParameters, 2, MPI_INT, i, PARAMETERS, MPI_COMM_WORLD);
        }
        array = (int *) _mm_malloc(sizeof(int) * numberOfArrays * arraySize, ALLOC_ALIGN_TRANSFER);
        for (int arrayCounter = 0; arrayCounter < numberOfArrays; arrayCounter++) {
            GenerateArray(array + (arrayCounter * arraySize), arraySize);
        }
    }
    else {
        int receiveParameters[2];
        MPI_Recv(receiveParameters, 2, MPI_INT, 0, PARAMETERS, MPI_COMM_WORLD, &status);
        arraySize = receiveParameters[0];
        numberOfArrays = receiveParameters[1];
    }

    if (me == 0) {
        int startingForProcess[numberOfProcesses - 1];
        std::vector<std::queue<Timer>> reqTimers;
        auto reqMutexs = new std::mutex[numberOfProcesses - 1];
        reqTimers.resize(numberOfProcesses - 1);
        static bool finishWork;
#pragma omp parallel private(status)
        {
#pragma omp sections
            {
#pragma omp section
                {
                    MPI_Request request[numberOfProcesses - 1];
                    MPI_Status statuses[numberOfProcesses - 1];
                    int actualStartingPoint = 0;
                    int finishStatus;
                    int howManyRequests = 0;
                    while (actualStartingPoint + arraySize <= numberOfArrays * arraySize) {
                        int ready = 1;
                        sendTimer.Start();
                        MPI_Recv(&ready, 1, MPI_INT, MPI_ANY_SOURCE, STATUS, MPI_COMM_WORLD, &status);
                        if (ready == 1 && (actualStartingPoint < numberOfArrays * arraySize)) {
                            startingForProcess[status.MPI_SOURCE - 1] = actualStartingPoint;
                            howManyRequests = howManyRequests % (numberOfProcesses-1);
                            MPI_Isend(&(array[actualStartingPoint]), arraySize, MPI_INT, status.MPI_SOURCE, STARTING,
                                     MPI_COMM_WORLD, &request[howManyRequests++]);
                            reqMutexs[status.MPI_SOURCE-1].lock();
                            reqTimers[status.MPI_SOURCE-1].push(sendTimer);
                            reqMutexs[status.MPI_SOURCE-1].unlock();
                            actualStartingPoint = actualStartingPoint + arraySize;
                        }
                    }
                    MPI_Waitall(howManyRequests, request, statuses);
                    finishStatus = 0;
                    for (int i = 1; i < numberOfProcesses; ++i)
                        MPI_Isend(&finishStatus, 1, MPI_INT, i, FINISH, MPI_COMM_WORLD, &request[i-1]);
                    MPI_Waitall(numberOfProcesses-1, request, statuses);
                    finishWork = true;
                }
#pragma omp section
                {
                    MPI_Request request[numberOfProcesses - 1];
                    MPI_Status statuses[numberOfProcesses - 1];
                    int howManyRequests = 0;
                    while (1) {
                        howManyRequests = howManyRequests % (numberOfProcesses-1);
                        MPI_Probe(MPI_ANY_SOURCE, RESULTS_DATA, MPI_COMM_WORLD, &status);
                        MPI_Irecv(&(array[startingForProcess[status.MPI_SOURCE - 1]]), arraySize, MPI_INT,
                                 status.MPI_SOURCE, RESULTS_DATA, MPI_COMM_WORLD, &request[howManyRequests++]);
                        reqMutexs[status.MPI_SOURCE-1].lock();
                        recvTimer = reqTimers[status.MPI_SOURCE-1].front();
                        reqTimers[status.MPI_SOURCE-1].pop();
                        reqMutexs[status.MPI_SOURCE-1].unlock();
                        recvTimer.Stop();
                        medianTimer.PushTime(recvTimer.Get());
                        if(finishWork)
                        {
                            MPI_Waitall(howManyRequests, request, statuses);
                            break;
                        }
                    }
                }
            }
        }
	delete[] reqMutexs;
    }
    else {
        int *smallArray = (int *) _mm_malloc(sizeof(int) * arraySize, ALLOC_ALIGN_TRANSFER);
        int finishStatus = 1;
        MPI_Request sendRequest =0, receiveRequest, statusRequest =0;
        MPI_Status singleStatus;
        MPI_Send(&finishStatus, 1, MPI_INT, 0, STATUS, MPI_COMM_WORLD);
        finishStatus = 0;
        while (true) {
            if(statusRequest != 0)
                MPI_Wait(&statusRequest, &singleStatus);
            if(sendRequest != 0)
                MPI_Wait(&sendRequest, &singleStatus);
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &singleStatus);
            if (singleStatus.MPI_TAG == STARTING) {
                MPI_Irecv(smallArray, arraySize, MPI_INT, 0, STARTING, MPI_COMM_WORLD, &receiveRequest);
                MPI_Wait(&receiveRequest, &singleStatus);

                MergeSort(smallArray, arraySize);
                MPI_Isend(smallArray, arraySize, MPI_INT, 0, RESULTS_DATA, MPI_COMM_WORLD, &sendRequest);

                finishStatus = 1;
                MPI_Isend(&finishStatus, 1, MPI_INT, 0, STATUS, MPI_COMM_WORLD, &statusRequest);
            }
            else if (singleStatus.MPI_TAG == FINISH) {
                break;
            }
        }
        _mm_free(smallArray);
    }
    if(me == 0)
    {
        _mm_free(array);
        mainTimerWithoutEnv.Stop();
        printf("MPI,%d,%d,%lu,%lu,%lu,",numberOfProcesses, arraySize, medianTimer.Get(), medianTimer.GetAvg(), mainTimerWithoutEnv.Get());
    }
    MPI_Finalize();
    mainTimer.Stop();
    if(me == 0) {
        printf("%lu,", mainTimer.Get());
    }
    return 0;
}
