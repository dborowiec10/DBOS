#include <math.h>

double cosh(double x){
  double dexpx;
  dexpx = exp(x);
  return(0.5 * (dexpx + (1.0 / dexpx)));
}
