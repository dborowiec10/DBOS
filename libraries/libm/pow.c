#include <math.h>

double pow(double x, double y){
    int j;
    int neg;
    double yy;
    double xx;

    neg = 0;
    j = y;
    yy = j;

    if(yy == y){

        xx = x;

        if(y < 0){
          neg = 1;
          j = -j;
        }

        if(y == 0){
          return (1.0);
        }

        --j;

        while(j>0){
          xx = xx * x;
          j--;
        }

        if(neg){
          xx = 1.0 / xx;
        }

        return xx;
    }

    if(x < 0.0){
         return 0.0;
    }

    if(y == 0.0){
      return (1.0);
    }

    return (exp(y * log(x)));
}
