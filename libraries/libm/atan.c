#include <math.h>

double atan(double x){
  int i;
  double term;
  double answer;
  double work;
  double powx;

  if(x < 0.0){
    return (-atan(-x));
  }

  if(x > 1.0){
    return ((PI / 2) - atan(1.0 / x));
  }

  if(x > (2.0 - sqrt(3.0))){
    return ((PI / 6.0) + atan((x * sqrt(3.0) -1.0 ) / (x + sqrt(3.0))));
  }

  i = 1;
  answer = x;
  term = x;
  powx = x;

  while (1){
      work = i;
      powx = 0.0 - powx * x * x;
      term = powx / (work + 2.0);
      if(answer == (answer + term)){
        break;
      }
      answer = answer + (term);
      i += 2;
  }

  return answer;
}
