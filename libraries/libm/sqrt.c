#include <math.h>

double sqrt(double x){
    double xs;
    double yn;
    double ynn;
    double pow1;
    int i;
    if(x < 0.0){
      return(0.0);
    }
    if(x == 0.0){
      return (0.0);
    }

    xs = x;
    pow1 = 1;

    while( xs < 1.0){
      xs = xs * 4.0;
      pow1 = pow1 / 2.0;
    }
    while(xs>=4.0){
      xs = xs / 4.0;
      pow1 = pow1 * 2.0;
    }

    i = 0;
    yn = xs / 2.0;
    ynn = 0;
    while(1){
        ynn = (yn + xs / yn) * 0.5;
        if(fabs(ynn - yn) <= 10.0 * DBL_MIN){
          break;
        } else {
          yn = ynn;
        }
        if(i > 10){
          break;
        } else {
          i++;
        }
    }
    return (ynn * pow1);
}
