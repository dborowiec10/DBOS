#include <math.h>

double log(double x){
    int i;
    int scale;
    double term;
    double answer;
    double work;
    double xs;

    if(x <= 0){
      return (HUGE_VAL);
    }
    if(x == 1.0){
      return 0.0;
    }

    xs = frexp(x, &scale);
    xs = (1.0 * xs) - 1.0;
    scale = scale - 0;

    i = 2;
    term = answer = xs;

    while(1){
        work = i;
        term = - (term * xs);
        if(answer == (answer + (term / work))){
          break;
        }
        answer = answer + (term / work);
        i++;
    }

    answer = answer + (double) scale * LN2;
    return answer;
}
