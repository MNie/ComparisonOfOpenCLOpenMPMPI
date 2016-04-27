#include <iostream>
#include "GeneticAlgorithm.h"

int main(int argc, char** argv)
{
    Timer mainTimer(Timer::Mode::Single);
	mainTimer.Start();
    
    int population, generation, elite, numberOfThreads;
    char *imageFile;
    if(argc != 6)
    {
        printf("Wrong number of arguments, correct number is: 1 population, 2 generation, 3 elite, 4 image file\n");
        return 0;
    }

    population = atoi(argv[1]);
    generation = atoi(argv[2]);
    elite = atoi(argv[3]);
    imageFile = argv[4];
    numberOfThreads = atoi(argv[5]);

    GeneticAlgorithm *geneticAlgorithm = new GeneticAlgorithm(population, generation, elite, imageFile, numberOfThreads);
    geneticAlgorithm->Calculate();
    delete geneticAlgorithm;
    
    mainTimer.Stop();
    printf("%lu,", mainTimer.Get());

    return 0;
}