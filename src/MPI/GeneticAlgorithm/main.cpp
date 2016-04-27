#include "GeneticAlgorithm.h"
#include <iostream>

int main(int argc, char** argv)
{
    Timer mainTimer(Timer::Mode::Single), mainTimerWithoutEnv(Timer::Mode::Single);
    mainTimer.Start();
    MPI_Init(&argc, &argv);
    mainTimerWithoutEnv.Start();
    int population, generation, elite, numberOfProcesses, commRank;
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
    char *imageFile;
    if(argc != 5)
    {
        printf("Wrong number of arguments, correct number is: 1 population, 2 generation, 3 elite, 4 image file\n");
        return 0;
    }

    population = atoi(argv[1]);
    generation = atoi(argv[2]);
    elite = atoi(argv[3]);
    imageFile = argv[4];
   
    GeneticAlgorithm *geneticAlgorithm = new GeneticAlgorithm(population, generation, elite, imageFile);
    geneticAlgorithm->Calculate();
    delete geneticAlgorithm;
    mainTimerWithoutEnv.Stop();
    if(commRank == 0)
        printf("%lu,", mainTimerWithoutEnv.Get());
    MPI_Finalize();
    mainTimer.Stop();
    if(commRank == 0)
        printf("%lu,", mainTimer.Get());
    return 0;
}