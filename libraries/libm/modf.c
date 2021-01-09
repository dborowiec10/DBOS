#include <math.h>

double modf(double x, double * e){
    int neg = 0;
    long i;
    if(x < 0){
      neg = 1;
      x = -x;
    }
    i = (long) x;
    x -= i;
    if(neg){
      x = -x;
      i = -i;
    }
    *e = i;

    return x;
}
