#ifndef COMPLEX_H
#define COMPLEX_H

#include <math.h>

class Complex
{
public:
	float Real;
	float Imaginary;

	Complex();

	Complex(float real, float imaginary);

	float BoundedOrbit(Complex *startingPoint, float bound, int bailout);
protected:
	Complex multiply(const Complex &other);
	
	float normalizedIterations(int n, int bailout);
	
	float length();
	
	Complex add(const Complex &other);

};

#endif //COMPLEX_H