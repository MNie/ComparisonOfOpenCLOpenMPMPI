#include "Complex.h"
#define FLT_MIN 0.00000001f


Complex Complex::multiply(const Complex &other)
{
	Complex returnComplex;
	returnComplex.Real = this->Real * other.Real - this->Imaginary * other.Imaginary;
	returnComplex.Imaginary = this->Imaginary * other.Real + this->Real * other.Imaginary;
	return returnComplex;
}

float Complex::normalizedIterations(int n, int bailout)
{
	return n + (log(log((float)(bailout))) - log(log(this->length()))) / log(2.0f);
}

float Complex::length()
{
	return sqrt(this->Real*this->Real + this->Imaginary * this->Imaginary);
}

Complex Complex::add(const Complex &other)
{
	Complex returnComplex;
	returnComplex.Real = this->Real + other.Real;
	returnComplex.Imaginary = this->Imaginary + other.Imaginary;
	return returnComplex;
}

Complex::Complex():Real(0.0f), Imaginary(0.0f)
{}

Complex::Complex(float real, float imaginary) : 
	Real(real), Imaginary(imaginary)
{}

float Complex::BoundedOrbit(Complex *startingPoint, float bound, int bailout)
{
	auto multiply = startingPoint->multiply(*startingPoint);
	auto z = this->add(multiply);
	float returnValue;
	for (int k = 0 ; k < bailout; ++k) {
		if (z.length() > bound)
		{
			returnValue = z.normalizedIterations(k, bailout);
			return returnValue;
		}
		multiply = z.multiply(z);
		z = this->add(multiply);
	}
	return FLT_MIN;
}
