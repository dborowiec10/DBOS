#include <math.h>

double atan2(double y, double x){
  return (x >= y) ? (x >= -y ? atan(y / x) : -PI / 2 - atan(x / y)) : (x >= -y ? PI / 2 - atan(x / y) : (y >= 0) ? PI + atan(y / x) : -PI + atan(y / x));
}
