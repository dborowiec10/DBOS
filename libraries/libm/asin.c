#include <math.h>

double asin(double y){
  int i;
  double term;
  double answer;
  double work;
  double x;
  double powx;
  double coef;

  x = y;
  if(x < 0.0){
    return (-asin(-x));
  }

  if(x > 1.0){
    return(HUGE_VAL);
  }

  if(x > 0.75){
    x = (sqrt(1.0 - (x * x)));
    return((PI / 2.0) - asin(x));
  }

  i = 1;
  answer = x;
  term = 1;
  coef = 1;
  powx = x;

  while (1){
    work = i;
    coef = (coef * work) / (work + 1);
    powx = powx * x * x;
    term = coef * powx / (work + 2.0);
    if(answer == (answer + term)){
      break;
    }
    answer = answer + (term);
    i += 2;
  }
  return answer;
}
