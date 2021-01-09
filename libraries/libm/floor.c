#include <math.h>

double floor(double x){
  int y;
  if (x < 0.0){
    y = (int) x;
    if ((double) y != x){
      y--;
    }
  } else {
    y = (int) x;
  }
  return (double) y;
}
