#include <math.h>

double tanh(double x){
    double dexp2;
    dexp2 = exp(-2.0 * x);
    return ((1.0 - dexp2) / (1.0 + dexp2));
}
