#include <math.h>

double fmod(double x, double y){
  int imod;
  if(y == 0.0){
    return 0.0;
  }
  imod = x / y;
  return ((double) x) -((double) imod * y);
}
