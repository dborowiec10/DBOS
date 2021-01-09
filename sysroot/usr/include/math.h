#ifndef MATH_H
#define MATH_H
#include <stdint.h>

#define PI 3.1415926535897932384626433832795
#define LN10 2.3025850929940456840179914546844
#define LN2 0.69314718055994530941723212145818
#define HUGE_VAL 9.999999999999999999999E72
#define DBL_MIN 1E-37

union dshape {
	double value;
	uint64_t bits;
};

double ceil(double x);
double fabs(double x);
double floor(double x);
double fmod(double x, double y);
double acos(double x);
double asin(double y);
double atan(double x);
double atan2(double y, double x);
double cos(double x);
double sin(double x);
double tan(double x);
double cosh(double x);
double sinh(double x);
double tanh(double x);
double exp(double x);
double frexp(double x, int * e);
double modf(double x, double * e);
double sqrt(double x);
double ldexp(double x, int n);
double pow(double x, double y);
double log(double x);
double log10(double x);
double log2(double x);
double cbrt(double x);
double trunc(double x);

#endif
