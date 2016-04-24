#include <iostream>
#include "GeneticAlgorithm.h"

int main(int argc, char** argv)
{
    int population, generation, elite;
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
    std::cout << "Bye!" << std::endl;
    delete geneticAlgorithm;

    return 0;
}