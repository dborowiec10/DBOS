#include <math.h>

double sin(double x){
  int i;
  double term;
  double answer;
  double work;
  double x1;

  i = x / (2.0 * PI);
  x1 =  x - (i * (2.0 * PI));

  i = 1;
  term = answer = x1;

  while (1){
    work = i+1;
    term = -(term * x1 * x1) / (work * (work + 1.0));
    if(answer == (answer + term)){
      break;
    }
    answer = answer + term;
    i = i + 2;
  }

  return(answer);
}
