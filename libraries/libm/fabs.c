#include <math.h>

double fabs(double x){
  if (x < 0.0){
    x = -x;
  }
  return x;
}
