#include <math.h>

double ceil(double x){
  int y = (int) x;
  if((double) y < x){
    y++;
  }
  return ((double) y);
}
