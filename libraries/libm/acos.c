#include <math.h>

double acos(double x){
  if(fabs(x) > 1.0){
    return (HUGE_VAL);
  }

  if(x < 0.0){
    return (PI - acos(-x));
  }

  return (asin(sqrt(1.0 - x * x)));
}
