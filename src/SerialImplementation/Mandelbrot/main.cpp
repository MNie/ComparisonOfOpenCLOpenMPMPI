#include "Structures/mandelbrot.h"
#include <chrono>
#include <vector>
#include "../../Libraries/Timer.h"


int main(int argc, char** argv)
{
	int width, height;
	Timer mainTimer(Timer::Mode::Median);
	if(argc != 3)
	{
		printf("%d", argc);
		printf("Wrong number of arguments, correct number is: 1- width, 2 height\n");
		return 0;
	}
	else
	{
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}
	mainTimer.Start();
	auto results = (char*)_mm_malloc(sizeof(char) * width * height, ALLOC_ALIGN);
	Mandelbrot(results, 0, width, height);
	mainTimer.Stop();
	stbi_write_png("mandelbrot_cl.png", width, height, 1, results, width);
	std::cout << mainTimer.Get() << std::endl;
	_mm_free(results);
	return 0;
}
