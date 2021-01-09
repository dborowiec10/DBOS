#include <math.h>

double exp(double x){
  int i;
  double term;
  double answer;
  double work;

  i = 2;
  term = x;
  answer = x;

  while(1){
    work = i;
    term = (term * x) / work;
    if(answer == (answer + term)){
      break;
    }
    answer = answer + (term);
    i++;
  }
  answer = answer + 1.0;

  return answer;
}
