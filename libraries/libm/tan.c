#include <math.h>

double tan(double x){
  double temp;

  temp = cos(x);
  if(temp == 0.0){
    return (HUGE_VAL);
  }

  return (sin(x) / cos(x));
}
